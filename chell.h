// CONSTANTS
#define CHELL_RL_BUFSIZE 1024
#define CHELL_TOK_BUFSIZE 64
#define CHELL_TOK_DELIM " \t\r\n\a"
#define CHELL_STDOUT_BUFSIZE 4096

#define CHELL_PROMPT_TIME_BUF_SIZE 6

// COLOR OUTPUT
#define BLU   "\x1B[34m"
#define GRN   "\x1B[32m"
#define YEL   "\x1B[33m"
#define GRY   "\x1B[37m"
#define CYA   "\x1B[36m"
#define BLD   "\033[1m"
#define RESET "\x1B[0m"


// FUNCTION PROTOTYPES
char *chell_read_line();
char **chell_split_line(char *);
char **chell_parse_line (char *);
char **chell_parse_cmd (char *);

void chell_prompt (void);

int chell_cd 	(char **args);
int chell_help 	(char **args);
int chell_halt 	(char **args);

int chell_num_builtins (void);

int  chell_launch(char **);
int chell_exec (char **args);
