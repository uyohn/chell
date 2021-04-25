#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <pwd.h>
#include <sys/wait.h>


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


// returns pipe success
// do not forget to close the pipe in parent
int pipe_exec (char **command) {
	int res = 0;

	int sout_pair[2];
	pid_t pid;
	int status;

	// error creating pipe
	if (pipe(sout_pair) < 0)
		return res;

	pid = fork();

	// error forking
	if (pid < 0)
		return res;

	// child
	if (pid == 0) {
		// redirect sout to pipe
		dup2(sout_pair[1], STDOUT_FILENO);
		// close pipe ends
		close(sout_pair[1]);
		close(sout_pair[0]); // read end not used in child

		// exec the command
		if ( execvp(command[0], command) == -1 )
			perror("chell");

		// stop child
		exit(127);
	}

	// parent
	do {
		// write end not used in parent
		close(sout_pair[1]);

		// copy read end of pipe
		res = sout_pair[0];

		// wait for child to finish
		waitpid(pid, &status, WUNTRACED);
	} while (!WIFEXITED(status) && !WIFSIGNALED(status));

	// return the read end of pipe (do not forget to close it in parent)
	return res;
}
