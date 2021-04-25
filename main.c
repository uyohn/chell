#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <argp.h>

#include "main.h"
#include "chell.h"

// ARGP setup
const char *argp_program_version = "chell 1.0";
const char *argp_program_bug_address = "<rastockymatej@gmail.com>";

// documentation
static char doc[] =
	"cHell\ncreated by Matej Rastocky\n\nsimple shell implemented in C\nsimple shell implemented in C\ndeveloped as a uni assignment\n\nheavily inspired by\nS. Brennan's tutorial\n\nthis shell only provides\nrudimentary functions\n\nuse help command to see builtins\b";

// args
static char args_doc[] = "";

// options
static struct argp_option options[] = {
	{"port",    'p', "PORT",      0,  "Local socket port" },
	{"path",    'u', "PATH",      0,  "Path to local socket" },
	{ 0 }
};

struct arguments {
	char *args[0];
	int port;
	char *path;
};

static error_t parse_opt (int key, char *arg, struct argp_state *state) {
	// get the input arg
	struct arguments *arguments = state->input;

	switch (key) {
		case 'p':
			arguments->port = atoi(arg);
			break;
		case 'u':
			arguments->path = arg;
			break;
		default:
			return ARGP_ERR_UNKNOWN;
	}

	return 0;
}

static struct argp argp = { options, parse_opt, args_doc, doc };

int main (int argc, char **argv) {
	// INICIALIZATION
	struct arguments arguments;
	arguments.port = -1;
	arguments.path = NULL;
	argp_parse(&argp, argc, argv, 0, 0, &arguments);

	printf("port: %d, path: %s\n", arguments.port, arguments.path);

	// clear terminal
	//system("clear");

	// handle ^C
	struct sigaction act;
	act.sa_handler = int_handler;
	sigaction(SIGINT, &act, NULL);



	// REPL LOOP
	chell_loop();



	// SHUTDOWN / CLEANUP
	return EXIT_SUCCESS;
}


// read, parse, execute
void chell_loop () {
	char *line;
	char **args;
	int status;

	do {
		chell_prompt(); 					// show the prompt

		// TODO: parsing
		line = chell_read_line();
		args = chell_split_line(line);
		status = chell_exec(args);

		free(line);
		free(args);

		printf("\n");
	} while (status);
}


// stop the (c)hell interrupt
void int_handler () {
	exit(EXIT_SUCCESS);
}
