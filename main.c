#include <stdio.h>
#include <stdlib.h>

#include "main.h"
#include "chell.h"


int main (int argc, char **argv) {
	// INICIALIZATION

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
		printf("> ");
		line = chell_read_line();
		args = chell_split_line(line);
		status = chell_exec(args);

		free(line);
		free(args);
	} while (status);
}
