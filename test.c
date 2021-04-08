#include <stdio.h>

int main () {
	printf("repeat chell\n");
	char *line = NULL;
	size_t bufsize = 0;
	int status = 1;

	do {
		printf("$ ");
		getline(&line, &bufsize, stdin);
		printf("%s\n", line);

	} while (status);

	return 0;
}
