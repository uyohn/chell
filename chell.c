#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "chell.h"


char *builtin_str[] = {
	"cd",
	"help",
	"exit"
};

int (*builtin_func[]) (char **) = {
	&chell_cd,
	&chell_help,
	&chell_exit
};

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


int chell_launch (char **args) {
	pid_t pid, wpid;
	int status;

	pid = fork();

	if (pid == 0) {
		// child process
		if (execvp(args[0], args) == -1) {
			perror("chell");
		}

		exit(EXIT_FAILURE);
	} else if (pid < 0) {
		// error forking
		perror("chell");
	} else {
		// parent process
		do {
			wpid = waitpid(pid, &status, WUNTRACED);
		} while (!WIFEXITED(status) && !WIFSIGNALED(status));
	}

	return 1;
}


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

int chell_num_builtins () {
	return sizeof(builtin_str) / sizeof (char *);
}

int chell_cd (char **args) {
	if (args[1] == NULL) {
		fprintf(stderr, "chell: expected arguments to \"cd\"\n");
	} else {
		if (chdir(args[1]) != 0) {
			perror("chell");
		}
	}

	return 1;
}


int chell_help (char **args) {
	printf("Stephen Brennan's tutorial\n");
	printf("Implemented builtins:\n");

	for (int i = 0; i < chell_num_builtins(); i++) {
		printf("\t%s\n", builtin_str[i]);
	}

	return 1;
}


int chell_exit (char **args) {
	return 0;
}
