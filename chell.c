#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <time.h>

#include "chell.h"
#include "helpers.h"


char *builtin_str[] = {
	"cd",
	"help",
	"halt"
};

int (*builtin_func[]) (char **) = {
	&chell_cd,
	&chell_help,
	&chell_halt
};

// print chell prompt
// TODO: make configurable by config file / env variables
void chell_prompt () {
	// get current time using format_time from helpers.c
	char time[CHELL_PROMPT_TIME_BUF_SIZE];
	get_current_time(time);

	// get info about user
	char *user_name = get_user_name();
	char *working_dir = get_working_dir();

	// get machine name
	char host_name[1024];
	host_name[1023] = '\0';
	gethostname(host_name, 1023);

	// get user name getpwuid
	printf(BLD "[%s] " CYA "%s" GRY " at " YEL BLD "%s" GRY "\n" GRN "%s" BLU " ≡ " RESET, time, user_name, host_name, working_dir);

	// cleanup
	free(working_dir);
}

// TODO: file as a parameter
char *chell_read_line (void) {
	char *line = NULL;
	size_t bufsize = 0;

	if ( getline(&line, &bufsize, stdin) == -1) {
		if ( feof(stdin) ) {
			exit(EXIT_SUCCESS);
		} else {
			perror("readline");
			exit(EXIT_FAILURE);
		}
	}

	return line;
}


// TODO: improve parsing
char **chell_split_line (char *line) {
	int bufsize = CHELL_TOK_BUFSIZE;
	int pos = 0;

	char **tokens = malloc(bufsize * sizeof(char*));
	char *token;

	if (!tokens) {
		fprintf(stderr, "chell: allocation error\n");
		exit(EXIT_FAILURE);
	}

	token = strtok(line, CHELL_TOK_DELIM);
	while (token != NULL) {
		tokens[pos] = token;
		pos++;

		if (pos >= bufsize) {
			bufsize += CHELL_TOK_BUFSIZE;
			tokens = realloc(tokens, bufsize * sizeof(char*));

			if (!tokens) {
				fprintf(stderr, "chell: allocation error\n");
				exit(EXIT_FAILURE);
			}
		}

		token = strtok(NULL, CHELL_TOK_DELIM);
	}

	tokens[pos] = NULL;
	return tokens;
}


// run a builtin function, takes precedence over chell_launch
int chell_exec (char **args) {
	// empty command
	if (args[0] == NULL)
		return 1;

	for (int i = 0; i < chell_num_builtins(); i++) {
		if (strcmp(args[0], builtin_str[i]) == 0) {
			return (*builtin_func[i])(args);
		}
	}

	return chell_launch(args);
}


// run a command in a child process
int chell_launch (char **args) {
	pid_t pid, wpid;
	int status;

	pid = fork();

	if (pid < 0) {
		perror("chell, forking");
		exit(EXIT_FAILURE);
	}

	// child process
	if (pid == 0) {
		if (execvp(args[0], args) == -1) {
			perror("chell");
		}

		exit(EXIT_FAILURE);
	} 
	
	// parent process
	do {
		wpid = waitpid(pid, &status, WUNTRACED);
	} while (!WIFEXITED(status) && !WIFSIGNALED(status));


	return 1;
}




// BUILTINS

int chell_num_builtins () {
	return sizeof(builtin_str) / sizeof (char *);
}

// change directory
int chell_cd (char **args) {
	// default: cd ~ if no arguments
	if (args[1] == NULL) {
		if (chdir(get_home_dir()) != 0)
			perror("chell");

		return 1;
	} 

	// cd if path argument specified
	if (chdir(args[1]) != 0)
		perror("chell");

	return 1;
}


// print info about author, program purpose and usage
int chell_help (char **args) {
	printf("\n////////  cHell  \\\\\\\\\\\\\\\\\n");
	printf("created by Matej Rastocky\n\n");


	printf("simple shell implemented in C\n");
	printf("developed as a uni assignment\n\n");

	printf("heavily inspired by\nS. Brennan's tutorial\n\n");

	printf("this shell only provides\nrudimentary functions\n\n");

	printf("implemented builtins:\n");

	for (int i = 0; i < chell_num_builtins(); i++) {
		printf("\t  %s\n", builtin_str[i]);
	}

	printf("\n");
	return 1;
}


// stop the REPL loop
int chell_halt (char **args) {
	return 0;
}
