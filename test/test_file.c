#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
	printf("Running test_file...\n");

	(void)unlink("file_syslog_test");

	/* Set SYSLOG_PATH to use a file. */
	setenv("SYSLOG_PATH", "file_syslog_test", 1);

	/* Write message using wrapped syslog(). */
	openlog("test", LOG_PID, LOG_USER);
	syslog(LOG_INFO, "test message");
	closelog();

	/* Read the message that was sent using wrapped syslog() from the file. */
	char buffer[1024];
	FILE *fp = fopen("file_syslog_test", "r");
	if (fp == NULL) {
		perror("fopen");
		exit(EXIT_FAILURE);
	}

	if (fgets(buffer, sizeof(buffer), fp) == NULL) {
		perror("fgets");
		exit(EXIT_FAILURE);
	}

	/* Verify the message. */
	printf("Received: %s", buffer);
	char *found = strstr(buffer, "test message");

	(void)unlink("file_syslog_test");

	/* Test verdict. */
	if (found) {
		printf("Success.\n");
		exit(EXIT_SUCCESS);
	} else {
		printf("Failure.\n");
		exit(EXIT_FAILURE);
	}
}
