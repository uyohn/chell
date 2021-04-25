#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <argp.h>

#include "main.h"
#include "chell.h"

// TODO: cleanup argp setup
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
	{"client",  'c',      0,      0,  "Open shell as a client" },
	{ 0 }
};

struct arguments {
	int port;
	char *path;
	int client;
	char *args[];
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
		case 'c':
			arguments->client = 1;
			break;
		default:
			return ARGP_ERR_UNKNOWN;
	}

	return 0;
}

static struct argp argp = { options, parse_opt, args_doc, doc };


// ENTRY POINT
int main (int argc, char **argv) {
	// INITIALIZATION
	// clear terminal
	system("clear");

	struct arguments arguments;
	arguments.port = -1;
	arguments.path = NULL;
	argp_parse(&argp, argc, argv, 0, 0, &arguments);

	// socket process
	if (arguments.port != -1 && arguments.path != NULL && arguments.client != 1)		
		open_socket(arguments.port, arguments.path);

	// client mode
	if (arguments.port != -1 && arguments.path != NULL && arguments.client == 1)
		conn_socket(arguments.port, arguments.path);


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
	char *prompt = (char *) malloc(CHELL_STDOUT_BUFSIZE * sizeof(char));

	char *line;
	char **commands;
	int status;

	int o_stdin = dup(STDIN_FILENO);

	do {
		chell_prompt(prompt);
		printf("%s", prompt); 					// show the prompt

		// get line
		line = chell_read_line();
		
		// parse it into commands
		commands = chell_parse_line(line);

		// exec commands
		status = chell_exec(commands);


		// cleanup
		free(line);
		free(commands);


		dup2(o_stdin, STDIN_FILENO);

		printf("\n");
	} while (status);

	close(o_stdin);
	free(prompt);
}


// stop the (c)hell interrupt
void int_handler () {
	exit(EXIT_SUCCESS);
}
