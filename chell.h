// CONSTANTS
#define CHELL_RL_BUFSIZE 1024
#define CHELL_TOK_BUFSIZE 64
#define CHELL_TOK_DELIM " \t\r\n\a"

// COLOR OUTPUT
#define BLU   "\x1B[34m"
#define GRN   "\x1B[32m"
#define YEL   "\x1B[33m"
#define RESET "\x1B[0m"


// FUNCTION PROTOTYPES
char *chell_read_line();
char **chell_split_line(char *);

void chell_prompt (void);

int chell_cd 	(char **args);
int chell_help 	(char **args);
int chell_halt 	(char **args);

int chell_num_builtins (void);

int  chell_launch(char **);
int chell_exec (char **args);
