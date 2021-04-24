
// CONSTANTS
#define CHELL_RL_BUFSIZE 1024
#define CHELL_TOK_BUFSIZE 64
#define CHELL_TOK_DELIM " \t\r\n\a"

// FUNCTION PROTOTYPES
char *chell_read_line();
char **chell_split_line(char *);
int  chell_launch(char **);

int chell_cd 	(char **args);
int chell_help 	(char **args);
int chell_halt 	(char **args);

int chell_num_builtins (void);
int chell_exec (char **args);
