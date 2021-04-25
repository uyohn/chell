#include <stdio.h>
#include <time.h>
#include <pwd.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#include "helpers.h"

// return formated time
void get_current_time (char *output) {
    time_t rawtime;
    struct tm * timeinfo;

    time ( &rawtime );
    timeinfo = localtime ( &rawtime );

    //sprintf(output, "[%d %d %d %d:%d:%d]",timeinfo->tm_mday, timeinfo->tm_mon + 1, timeinfo->tm_year + 1900, timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);

    sprintf(output, "%02d:%02d", timeinfo->tm_hour, timeinfo->tm_min);
}


// return user name
char *get_user_name () {
	uid_t id = getuid();
	struct passwd *pwd;

	pwd = getpwuid(id);

	return pwd->pw_name;
}


// return working dir
char *get_working_dir () {
	char *buf = malloc(FILENAME_MAX * sizeof(char));

	getcwd( buf, FILENAME_MAX );

	return buf;
}


// return user home dir
char *get_home_dir () {
	uid_t id = getuid();
	struct passwd *pwd;

	pwd = getpwuid(id);

	return pwd->pw_dir;
}



// custom implementation of popen
stdpipes chell_popen (const char *command) {
	stdpipes result = {.out = NULL, .in = NULL, .err = NULL};

	int sout_pair[2];
	pid_t pid;

	if (pipe(sout_pair) < 0)
		return result;


	pid = fork();

	// check for errors
	if (pid < 0) {
		close(sout_pair[0]);
		close(sout_pair[1]);
		return result;
	}

	// child
	if (pid == 0) {
		close(sout_pair[0]);
		dup2(sout_pair[1], STDOUT_FILENO);
		close(sout_pair[1]);

		execl("/bin/sh", "sh", "-c", command, NULL);
		exit(127);
	}

	//parent
	close(sout_pair[1]);
	result.out = fdopen(sout_pair[0], "r");

	return result;
}
