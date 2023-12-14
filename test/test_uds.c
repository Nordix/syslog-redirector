#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <syslog.h>
#include <unistd.h>

int create_socket(char *path) {
	int uds_sock;

	if ((uds_sock = socket(AF_UNIX, SOCK_DGRAM, 0)) == -1) {
		perror("socket");
		return -1;
	}

	struct sockaddr_un addr = {
		.sun_family = AF_UNIX,
		.sun_path = {0},
	};
	strncpy(addr.sun_path, path, sizeof(addr.sun_path) - 1);

	if (bind(uds_sock, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
		perror("bind");
		return -1;
	}

	return uds_sock;
}

int main(int argc, char *argv[]) {
	printf("Running test_uds...\n");

	/* Create socket to emulate local syslog server. */
	(void)unlink("uds_syslog_test");
	int uds_sock = create_socket("uds_syslog_test");
	if (uds_sock == -1) {
		exit(EXIT_FAILURE);
	}

	/* Set SYSLOG_PATH to use the UNIX domain socket. */
	setenv("SYSLOG_PATH", "unix:uds_syslog_test", 1);

	/* Write message using wrapped syslog(). */
	openlog("test", LOG_PID, LOG_USER);
	syslog(LOG_INFO, "test message");
	closelog();

	/* Receive the message that was sent using wrapped syslog() from the socket.
	 */
	char buffer[1024];
	int numbytes = recv(uds_sock, buffer, sizeof(buffer), 0);
	if (numbytes == -1) {
		perror("recv");
		exit(EXIT_FAILURE);
	}

	/* Verify the message. */
	printf("Received: %s", buffer);
	char *found = strstr(buffer, "test message");

	(void)unlink("uds_syslog_test");

	/* Test verdict. */
	if (found) {
		printf("Success.\n");
		exit(EXIT_SUCCESS);
	} else {
		printf("Failure.\n");
		exit(EXIT_FAILURE);
	}
}
