/**
 * This file is originally from musl libc.
 *
 * Copyright © 2005-2020 Rich Felker, et al.
 * Copyright (c) 2023 OpenInfra Foundation Europe and others.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <syslog.h>
#include <time.h>
#include <unistd.h>

#include "lock.h"

LOCK_INIT(lock)

enum { LOG_TYPE_UNIX = 1, LOG_TYPE_FILE = 2 };

static char log_ident[32];
static int log_opt;
static int log_facility = LOG_USER;
static int log_mask = 0xff;
static int log_fd = -1;
static int log_type = LOG_TYPE_UNIX;

int setlogmask(int maskpri) {
	LOCK(lock);
	int ret = log_mask;
	if (maskpri)
		log_mask = maskpri;
	UNLOCK(lock);
	return ret;
}

static struct sockaddr_un log_addr = {AF_UNIX, "/dev/log"};

void closelog(void) {
	int cs;
	pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &cs);
	LOCK(lock);
	close(log_fd);
	log_fd = -1;
	UNLOCK(lock);
	pthread_setcancelstate(cs, 0);
}

static void __openlog() {
	char *path = getenv("SYSLOG_PATH");
	char *file_path;

	if (path) {
		if (strncmp(path, "unix:", 5) == 0) {
			strncpy(log_addr.sun_path, path + 5, sizeof log_addr.sun_path - 1);
		} else if (strncmp(path, "file:", 5) == 0) {
			file_path = path + 5;
			log_type = LOG_TYPE_FILE;
		} else {
			file_path = path;
			log_type = LOG_TYPE_FILE;
		}
	}

	if (log_type == LOG_TYPE_UNIX) {
		log_fd = socket(AF_UNIX, SOCK_DGRAM | SOCK_CLOEXEC, 0);
		if (log_fd >= 0)
			connect(log_fd, (void *)&log_addr, sizeof log_addr);
	} else {
		log_fd =
			open(file_path, O_WRONLY | O_APPEND | O_CREAT | O_CLOEXEC, 0644);
	}
}

void openlog(const char *ident, int opt, int facility) {
	int cs;
	pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &cs);
	LOCK(lock);

	if (ident) {
		size_t n = strnlen(ident, sizeof log_ident - 1);
		memcpy(log_ident, ident, n);
		log_ident[n] = 0;
	} else {
		log_ident[0] = 0;
	}
	log_opt = opt;
	log_facility = facility;

	if ((opt & LOG_NDELAY) && log_fd < 0)
		__openlog();

	UNLOCK(lock);
	pthread_setcancelstate(cs, 0);
}

static int is_lost_conn(int e) {
	return e == ECONNREFUSED || e == ECONNRESET || e == ENOTCONN || e == EPIPE;
}

static void _vsyslog(int priority, const char *message, va_list ap) {
	char timebuf[16];
	time_t now;
	struct tm tm;
	char buf[1024];
	int errno_save = errno;
	int pid;
	int l, l2;
	int hlen;
	int fd;

	if (log_fd < 0)
		__openlog();

	if (!(priority & LOG_FACMASK))
		priority |= log_facility;

	now = time(NULL);
	gmtime_r(&now, &tm);
	strftime(timebuf, sizeof timebuf, "%b %e %T", &tm);

	pid = (log_opt & LOG_PID) ? getpid() : 0;
	l = snprintf(buf, sizeof buf, "<%d>%s %n%s%s%.0d%s: ", priority, timebuf,
				 &hlen, log_ident, "[" + !pid, pid, "]" + !pid);
	errno = errno_save;
	l2 = vsnprintf(buf + l, sizeof buf - l, message, ap);
	if (l2 >= 0) {
		if (l2 >= sizeof buf - l)
			l = sizeof buf - 1;
		else
			l += l2;
		if (buf[l - 1] != '\n')
			buf[l++] = '\n';

		if (log_type == LOG_TYPE_UNIX) {
			if (send(log_fd, buf, l, 0) < 0 &&
				(!is_lost_conn(errno) ||
				 connect(log_fd, (void *)&log_addr, sizeof log_addr) < 0 ||
				 send(log_fd, buf, l, 0) < 0) &&
				(log_opt & LOG_CONS)) {
				fd = open("/dev/console", O_WRONLY | O_NOCTTY | O_CLOEXEC);
				if (fd >= 0) {
					dprintf(fd, "%.*s", l - hlen, buf + hlen);
					close(fd);
				}
			}
			if (log_opt & LOG_PERROR)
				dprintf(2, "%.*s", l - hlen, buf + hlen);
		} else {
			dprintf(log_fd, "%.*s", l - hlen, buf + hlen);
		}
	}
}

static void __vsyslog(int priority, const char *message, va_list ap) {
	int cs;
	if (!(log_mask & LOG_MASK(priority & 7)) || (priority & ~0x3ff))
		return;
	pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &cs);
	LOCK(lock);
	_vsyslog(priority, message, ap);
	UNLOCK(lock);
	pthread_setcancelstate(cs, 0);
}

void syslog(int priority, const char *message, ...) {
	va_list ap;
	va_start(ap, message);
	__vsyslog(priority, message, ap);
	va_end(ap);
}

void vsyslog(int priority, const char *message, va_list ap)
	__attribute__((weak, alias("__vsyslog")));

void __syslog_chk(int priority, int flag, const char *message, ...) {
	va_list ap;
	va_start(ap, message);
	__vsyslog(priority, message, ap);
	va_end(ap);
}

void __vsyslog_chk(int priority, int flag, const char *message, va_list ap) {
	__vsyslog(priority, message, ap);
}
