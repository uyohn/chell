#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <time.h>
#include <sys/socket.h>
#include <sys/un.h>

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
void chell_prompt (char *str) {
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
	sprintf(str, BLD "[%s] " CYA "%s" GRY " at " YEL BLD "%s" GRY "\n" GRN "%s" BLU " ≡ " RESET, time, user_name, host_name, working_dir);

	// cleanup
	free(working_dir);
}


// read a line from stdin
char *chell_read_line (void) {
	char *line = NULL;
	size_t bufsize = 0;

	if ( getline(&line, &bufsize, stdin) == -1 ) {
		if ( feof(stdin) )
			exit(EXIT_SUCCESS);

		perror("readline");
		exit(EXIT_FAILURE);
	}

	return line;
}


// parse line into commands (split by | (pipe))
char **chell_parse_line (char *line) {
	size_t bufsize = CHELL_TOK_BUFSIZE;
	int pos = 0;

	char **tokens = malloc(bufsize * sizeof(char*));
	char *token;

	if (!tokens) {
		fprintf(stderr, "chell: allocation error\n");
		exit(EXIT_FAILURE);
	}

	token = strtok(line, "|");
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

		token = strtok(NULL, "|");
	}

	tokens[pos] = NULL;

	return tokens;
}

// parse command into tokens separated by whitespace
char **chell_parse_cmd (char *command) {
	size_t bufsize = CHELL_TOK_BUFSIZE;
	int pos = 0;

	char **tokens = malloc(bufsize * sizeof(char*));
	char *token;

	if (!tokens) {
		fprintf(stderr, "chell: allocation error\n");
		exit(EXIT_FAILURE);
	}

	token = strtok(command, CHELL_TOK_DELIM);
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
int chell_exec (char **commands) {
	// parse the first command into args array
	char **args = chell_parse_cmd(commands[0]);

	// empty command
	if (args[0] == NULL)
		return 1;


	// try to find the command in builtins
	for (int i = 0; i < chell_num_builtins(); i++) {
		if (strcmp(args[0], builtin_str[i]) == 0) {
			return (*builtin_func[i])(args);
		}
	}

	free(args);


	// if not found, launch the command chain
	return chell_launch(commands);
}


// run command/s in a child process
int chell_launch (char **commands) {
	char *command = NULL;

	// for each cmd
	for (int i = 0; (command = commands[i]) != NULL; i++) {

		// split command into tokens
		char **tokens = chell_parse_cmd(command);

		// prepare pipe
		if (commands[i+1] != NULL) {
			// exec the command
			int sout_fd = pipe_exec(tokens);

			// redirect last command out to next in
			dup2(sout_fd, STDIN_FILENO);
			close(sout_fd);

			continue;
		}

		// last command in pipe chain - print its output
		int sout_fd = pipe_exec(tokens);
		FILE *sout = fdopen(sout_fd, "r");

		char buffer[CHELL_STDOUT_BUFSIZE];

		// print sout from last command
		while ( fgets(buffer, CHELL_STDOUT_BUFSIZE, sout) )
			printf("%s", buffer);
	}

	return 1;
}


// SOCKET
void open_socket (int port, char *path) {
	pid_t pid = fork();

	if (pid == 0) {
		// prepare prompt string
		char *prompt = (char *) malloc(CHELL_STDOUT_BUFSIZE * sizeof(char));

		struct sockaddr_un addr;

		memset(&addr, 0, sizeof(addr));

		addr.sun_family = AF_UNIX;
		strcpy(addr.sun_path, path);

		// s is "main" socket, it is listening
		int s = socket(AF_UNIX, SOCK_STREAM, 0);

		if (s == -1) {
			perror("socket");
			exit(EXIT_FAILURE);
		}

		// if socket exists, delete it
		unlink(path);

		int b = bind(s, (struct sockaddr *) &addr, sizeof(addr));

		if (b == -1) {
			perror("bind");
			exit(EXIT_FAILURE);
		}

		// listen
		listen(s, 1);
		printf("server is listening on port %d and path %s\n", port, path);

		// only 1 client
		int client_fd = accept(s, NULL, NULL);

		if (client_fd < 0) {
			perror("conn failed");
			exit(EXIT_FAILURE);
		}

		printf("Client connected\n");
		// show prompt
		chell_prompt(prompt);
		send(client_fd, prompt, strlen(prompt), 0);


		int r;
		char buf[64];

		char **commands;

		dup2(client_fd, STDOUT_FILENO);

		while( (r = read(client_fd, buf, 64)) > 0) {
			buf[r] = 0;          // za poslednym prijatym znakom

			// parse
			commands = chell_parse_line(buf);
			//exec
			chell_exec(commands);

			// show prompt
			chell_prompt(prompt);
			send(client_fd, prompt, strlen(prompt), 0);
		}

		free(prompt);
		exit(EXIT_SUCCESS);
	}

	return;
}

// CLIENT MODE
void conn_socket (int port, char *path) {
	// connect to the socket
	struct sockaddr_un addr;

	memset(&addr, 0, sizeof(addr));

	addr.sun_family = AF_UNIX;
	strcpy(addr.sun_path, path);

	// s is "main" socket, it is listening
	int s = socket(AF_UNIX, SOCK_STREAM, 0);

	if (s == -1) {
		perror("socket");
		exit(EXIT_FAILURE);
	}

	int c = connect(s, (struct sockaddr *) &addr, sizeof(addr));

	if (c == -1) {
		perror("connect");
		exit(EXIT_FAILURE);
	}

	printf("Connected to local socket %s on port %d\n\n", path, port);


	// client read from stdin and server
	int r;
	fd_set rs;
	char msg[64] = "hellp";

	FD_ZERO(&rs);
	FD_SET(0, &rs);
	FD_SET(s, &rs);

	// toto umoznuje klientovi cakat na vstup z terminalu (stdin) alebo zo soketu
	// co je prave pripravene, to sa obsluzi (nezalezi na poradi v akom to pride)
	while( select(s + 1, &rs, NULL, NULL, NULL) > 0) {
		if (FD_ISSET(s, &rs)) {  				// je to deskriptor s - soket spojenia na server?
			r = read(s, msg, 1); 			  	// precitaj zo soketu (od servera)
			write(STDOUT_FILENO, msg, r);    	// zapis na deskriptor 1 = stdout (terminal)
		}

		if (FD_ISSET(0, &rs)) {
			r = read(STDIN_FILENO, msg, 64);  	// precitaj zo stdin (terminal)
							   					//if (msg[r-1]=='\n') msg[r-1]=0;
			write(s, msg, r);   				// posli serveru (cez soket s)
		}

		FD_ZERO(&rs);          					// connect() mnoziny meni, takze ich treba znova nastavit
		FD_SET(STDIN_FILENO, &rs);
		FD_SET(s, &rs);
	}

	perror("select");        		// ak server skonci, nemusi ist o chybu
	close(s);
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
