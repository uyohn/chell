#include <stdio.h>

typedef struct {
	FILE *in, *out, *err;
} stdpipes;

void  get_current_time (char *);
char *get_user_name    (void);
char *get_working_dir  (void);
char *get_home_dir     (void);

int pipe_exec (char **);
