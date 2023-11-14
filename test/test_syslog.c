#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
	/* Write message using wrapped syslog(). */
	openlog("test", LOG_PID, LOG_USER);
	syslog(LOG_INFO, "test message");
	closelog();
}
