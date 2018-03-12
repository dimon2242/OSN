#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/unistd.h>
#include <string.h>
#include <fcntl.h>

#define TRUE 1
#define FALSE 0
#define MAXARGS 16

#define PERMS 0666

#define STDIN_FILE0 0
#define STDOUT_FILE0 1
#define REDIR_IN 0
#define REDIR_OUT 1
#define NOREDIR 2

int main() {
	char separators[] = {' ', '\n', '<', '>', '\0'};
	while(TRUE) {
		char *argv[MAXARGS + 1];
		char *stringBuffer = NULL;
		int buffer;
		int lineIterator = -1;
		unsigned int charPosition = 0;
		int iFd = -1;
		int oFd = -1;
		short alreadySpaced = FALSE;
		short redirectionState = NOREDIR; // State of redirection
		char *inputFilename = NULL; // IO File name
		char *outputFilename = NULL;

		putchar('>');

		while(lineIterator < MAXARGS) {
			buffer = getchar();
			if(buffer == EOF) {
				putchar('\n');
				return EXIT_SUCCESS;
			}
			if((buffer == ' ') && alreadySpaced) {
				continue;
			} else if((buffer != ' ') && (buffer != '>') && (buffer != '<') && alreadySpaced) {
				alreadySpaced = FALSE;
			} else if((buffer == ' ') || (buffer == '>') || (buffer == '<')) {
				alreadySpaced = TRUE;
				if(buffer == '>') {
					redirectionState = REDIR_OUT;
				} else if(buffer == '<') {
					redirectionState = REDIR_IN;
				}
			}
			if(strrchr(separators, buffer) == NULL) {
				/*
				charPosition + 2 for ended null-terminator
				*/
				stringBuffer = (char*) realloc(stringBuffer, (charPosition + 2) * sizeof(char));
				if(stringBuffer == NULL) {
					perror("Error allocation of memory");
					return EXIT_FAILURE;
				}
				stringBuffer[charPosition] = buffer;
			} else {
				
				if(charPosition != 0) {
					*(stringBuffer + charPosition) = '\0';			
					if((redirectionState != NOREDIR) && !alreadySpaced) {
						if(redirectionState == REDIR_OUT) {
							outputFilename = (char*) malloc((charPosition + 2) * sizeof(char));
							strcpy(outputFilename, stringBuffer);
						} else if(redirectionState == REDIR_IN) {
							inputFilename = (char*) malloc((charPosition + 2) * sizeof(char));
							strcpy(inputFilename, stringBuffer);
						}
						redirectionState = NOREDIR;
					} else {
						argv[++lineIterator] = (char*) malloc((charPosition + 1) * sizeof(char));
						strcpy(argv[lineIterator], stringBuffer);
					}
					charPosition = 0;
					free(stringBuffer);
					stringBuffer = NULL;

					if(buffer == '\n') {
						argv[lineIterator + 1] = NULL;
						break;
					}
				}
				continue;
			}
			charPosition++;
		}

		pid_t pid = fork();
		if(!pid) {
			if(inputFilename != NULL) {
				if((iFd = open(inputFilename, O_RDONLY)) != -1) {

					if(dup2(iFd, STDIN_FILE0) == -1) {
						perror("dup2 in");
						return EXIT_FAILURE;
					}
				}
			}
			if(outputFilename != NULL) {
				if((oFd = open(outputFilename, O_WRONLY | O_CREAT | O_TRUNC, PERMS)) != -1) {
					if(dup2(oFd, STDOUT_FILE0) == -1) {
						perror("dup2 out");
						return EXIT_FAILURE;
					}
				}
			}
			if(execvp(*argv, argv) == -1) {
				perror("execvp error");
			}
		}

		pid = wait(NULL);
		if(pid == -1) {
			perror("wait error");
		}

		for(int i = 0; i <= lineIterator; i++) {
			free(argv[i]);
		}
		free(inputFilename);
		free(outputFilename);

	}
}
