#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>

int main(int argc, char *argv[]) {
	openlog("writer", LOG_PID, LOG_USER);
	if (argc != 3) {
		syslog(LOG_ERR, "Error: need more arguments : writer <file> <text>");
		closelog();
		exit(1);
	}

	const char *file = argv[1];
	const char *text = argv[2];

	FILE *fp = fopen(file, "w");
	if (fp == NULL) {
		syslog(LOG_ERR, "Error: can not open %s", file);
		closelog();
		exit(1);
	}
	fprintf(fp, "%s", text);
	fclose(fp);
	syslog(LOG_DEBUG, "Writing '%s' to '%s'", text, file);
	closelog();
	return 0;
}
