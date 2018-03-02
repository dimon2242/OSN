#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/unistd.h>
#include <string.h>

#define TRUE 1

int main() {
	while(TRUE) {
		char *argv[17], buffer[80], nl;
		int i = 0, state = 0;
		putchar('>');
		do {
			state = scanf("%79s%c", buffer, &nl);
			if((state == EOF) || (i == 15)) {
				argv[i] = NULL;
				break;			
			}
			argv[i] = (char*) malloc(80*sizeof(char));
			strcpy(argv[i], buffer);
			if(nl == '\n') {
				argv[i+1] = NULL;
				break;
			}
			i++;
		} while(state != EOF);

		if(state == EOF) {
			for(int j = 0; j <= i; j++)
				free(argv[j]);
			putchar('\n');
			return EXIT_SUCCESS;
		}

		pid_t pid = fork();
		if(!pid) {
			if(execvp(*argv, argv) == -1) {
				perror("execvp error");
				return EXIT_FAILURE;
			}

		}
		pid = wait(NULL);
		if(pid == -1) {
			perror("wait error");
			for(int j = 0; j <= i; j++)
				free(argv[j]);
			return EXIT_FAILURE;
		}

		for(int j = 0; j <= i; j++)
				free(argv[j]);
	}
}

