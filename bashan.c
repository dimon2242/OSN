#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/unistd.h>
#include <string.h>

#define TRUE 1
#define MAXLENGTH 80
#define MAXARGS 16

int main() {
	while(TRUE) {
		char *argv[MAXARGS + 1];
		char buffer[MAXLENGTH]; // + 1 for NULL pointer
		char specSymbol;
		int lines = -1; // -1 is no lines
		int state = 0;
		putchar('>');
		do {
			state = scanf("%79s%c", buffer, &specSymbol);
			if(state == EOF) {
				for(int j = 0; j <= lines; j++) {
					free(argv[j]);
				}
				putchar('\n');
				return EXIT_SUCCESS;
			}

			if(lines == (MAXARGS-1)) {
				argv[lines+1] = NULL;
				break;			
			}

			argv[++lines] = (char*) malloc(state * sizeof(char));
			strcpy(argv[lines], buffer);
			if(specSymbol == '\n') {
				argv[lines+1] = NULL;
				break;
			}
			
		} while(state != EOF);

		pid_t pid = fork();
		if(!pid) {
			if(execvp(*argv, argv) == -1) {
				perror("execvp error");
			}
		}

		pid = wait(NULL);
		if(pid == -1) {
			perror("wait error");
		}

		for(int j = 0; j <= lines; j++) {
			free(argv[j]);
		}
	}
}

