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
	char separators[] = {' ', '\n', '<', '>', '|', '\0'};
	while(TRUE) {
		char *argv[MAXARGS + 1];
		char *argvAdditional[MAXARGS + 1]; // For second process
		char *stringBuffer = NULL;
		char *tempPtr;
		int buffer;
		int memMultiplier = 1;
		int lineIterator = -1;
		int additionalLineIterator = -1;
		unsigned int charPosition = 0;
		int iFd = -1;
		int oFd = -1;
		short alreadySpaced = FALSE;
		short redirectionState = NOREDIR; // State of redirection
		char *inputFilename = NULL; // IO File name
		char *outputFilename = NULL;
		char *genericFilename = NULL; // Generic file name for second process
		int pipeIsEnabled = FALSE;
		int pipefd[2];

		putchar('>');

		while(lineIterator < MAXARGS) {
			buffer = getchar();
			if(buffer == EOF) {
				putchar('\n');
				return EXIT_SUCCESS;
			}
			if((buffer == ' ') && alreadySpaced) {
				continue;
			} else if((buffer != ' ') && (buffer != '>') && (buffer != '<') && (buffer != '|') && alreadySpaced) {
				alreadySpaced = FALSE;
			} else if((buffer == ' ') || (buffer == '>') || (buffer == '<') || (buffer == '|')) {
				alreadySpaced = TRUE;
			}
			if(strrchr(separators, buffer) == NULL) {
				if(((memMultiplier - 1) == charPosition)) {
					memMultiplier *= sizeof(char) * 2;
					tempPtr = (char*) realloc(stringBuffer, memMultiplier);
					if(tempPtr == NULL) {
						perror("Error allocation of memory");
						return EXIT_FAILURE;
					}
					stringBuffer = tempPtr;
					tempPtr = NULL;
				}
				*(stringBuffer + charPosition) = buffer;
			} else {
				if(charPosition != 0) {
					*(stringBuffer + charPosition) = '\0';			
					if((redirectionState != NOREDIR)) {
						if(pipeIsEnabled) {
							genericFilename = stringBuffer;
						} else if(redirectionState == REDIR_OUT) {
							outputFilename = stringBuffer;
						} else if(redirectionState == REDIR_IN) {
							inputFilename = stringBuffer;
						}
						redirectionState = NOREDIR;
					} else {
						if(!pipeIsEnabled) {
							argv[++lineIterator] = stringBuffer;
							argv[lineIterator + 1] = NULL;
						} else {
							argvAdditional[++additionalLineIterator] = stringBuffer;
							argvAdditional[additionalLineIterator + 1] = NULL;
						}
					}
					charPosition = 0;
					memMultiplier = 1;
					stringBuffer = NULL;
					if(buffer == '\n') {
						break;
					}
				}
				if(buffer == '|') {
					pipeIsEnabled = TRUE;
				} else if(buffer == '>') {
					redirectionState = REDIR_OUT;
				} else if(buffer == '<') {
					redirectionState = REDIR_IN;
				}
				continue;
			}
			charPosition++;
		}

		if(pipeIsEnabled) {
			if(pipe(pipefd) == -1) {
				perror("pipe");
				return EXIT_FAILURE;
			}
		}
		pid_t pid1 = fork();
		if(!pid1) {
			if(pipeIsEnabled) {
				close(pipefd[0]);
				if(dup2(pipefd[1], STDOUT_FILE0) == -1) {
					perror("dup2 pipefd[0]");
					return EXIT_FAILURE;
				}
			} else {
				if(outputFilename != NULL) {
					if((oFd = open(outputFilename, O_WRONLY | O_CREAT | O_TRUNC, PERMS)) != -1) {
						if(dup2(oFd, STDOUT_FILE0) == -1) {
							perror("dup2 out");
							return EXIT_FAILURE;
						}
					} else {
						perror("open error");
						return EXIT_FAILURE;
					}
				}
			}


			if(inputFilename != NULL) {
				if((iFd = open(inputFilename, O_RDONLY)) != -1) {

					if(dup2(iFd, STDIN_FILE0) == -1) {
						perror("dup2 in");
						return EXIT_FAILURE;
					}
				} else {
					perror("open error");
					return EXIT_FAILURE;
				}
			}
			
			if(execvp(*argv, argv) == -1) {
				perror("execvp error");
				return EXIT_FAILURE;
			}
		}
		pid_t pid2;
		if(pipeIsEnabled) {
			pid2 = fork();
			if(!pid2) {
				if(pipeIsEnabled) {
					close(pipefd[1]);
					if(dup2(pipefd[0], STDIN_FILE0) == -1) {
						perror("dup2 pipefd[1]");
						return EXIT_FAILURE;
					}
				}
				if(genericFilename != NULL) {
					if((oFd = open(genericFilename, O_WRONLY | O_CREAT | O_TRUNC, PERMS)) != -1) {
						if(dup2(oFd, STDOUT_FILE0) == -1) {
							perror("dup2 out");
							return EXIT_FAILURE;
						}
					} else {
						perror("open error");
						return EXIT_FAILURE;
					}
				}
				if(execvp(*argvAdditional, argvAdditional) == -1) {
					perror("execvp error");
					return EXIT_FAILURE;
				}
			}
		}
		if(pipeIsEnabled) {
			close(pipefd[0]);
			close(pipefd[1]);
			wait(NULL);
		}
		wait(NULL);

		for(int i = 0; i < lineIterator + 1; i++) {
			free(argv[i]);
		}
		for(int i = 0; i < additionalLineIterator + 1; i++) {
			free(argvAdditional[i]);
		}
		free(inputFilename);
		free(outputFilename);
		if(pipeIsEnabled && (genericFilename != NULL)) {
			free(genericFilename);
		}
		close(oFd);
		close(iFd);
	}
}
