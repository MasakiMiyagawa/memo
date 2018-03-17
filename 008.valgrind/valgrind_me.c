#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <libgen.h>
#include <sys/types.h>

#define VLOG_PATH "."
#define ARG_MAX_FORCE 128
int32_t ValgrindMe(int argc, char *argv[])
{
	int32_t i = 0;
	int32_t j = 0;
	char *_argv[ARG_MAX_FORCE];
	char logfile[256] = {0};
	int32_t err = 0;

	if (getenv("VALGRINDPROC")) {
		unsetenv("VALGRINDPROC");
		return 0;
	}
	
	err = snprintf(logfile, sizeof(logfile), 
			"--xml-file=%s/valgrind_%s_%d.xml", 
			VLOG_PATH, basename(argv[0]), getpid());
	if (err < 0) {
		return -1;
	}

	/* Set valgrind's args */
	_argv[i++] = "/usr/bin/valgrind";
	_argv[i++] = "--tool=memcheck";
	_argv[i++] = "--leak-check=full";
	_argv[i++] = "--xml=yes";
	_argv[i++] = logfile;
	_argv[i++] = "--trace-children=yes";

	/* Check buffer overflow */
	if ((i + argc) >= ARG_MAX_FORCE) {
		return -1;
	}

	/* Copy original args */
	for (j = 0; j < argc; j++, i++) {
		_argv[i] = argv[j];
	}

	/* Terminator should be NULL */
	_argv[i] = NULL;

	setenv("VALGRINDPROC", "1", 1);
	execvp(_argv[0], _argv);
}
