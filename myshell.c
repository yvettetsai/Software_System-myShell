/*
 * 
 * Name: myshell.c
 * 
 * Written by:
 *   Cody Doucette
 *   Yvette Tsai
 * 
 * Description:
 *   This program implement a basic shell called "myshell" which
 *   act just like the basic bash shell. 
 * 
 * Sample interaction:
 *   cmd			A sigle cmd that identifies the name of a program file
 *   cmd; cmd			A sequence of cmd which "myshell" will execture one after another
 *   cmd > output_file		Redirect stdout of cmd to the file named output_file
 *   cmd < input_file		Redirect stdin of cmd to the file named input_file
 *   cmd 1> output_file 	Redirect stdout of cmd to the file named output_file 
 *   cmd 2> output_file		Redirect stderr of cmd to the file named output_file
 *   cmd &> outpout_file 	Redirect both stdout and stderr to the file named output_file
 *   cmd1 | cmd2 | cmd3  	Pipeline the output of the cmd before each "|" symbol to the 
 *					input of the cmd after the same "|" symbol
 *   cmd &			
 "myshell" let cmd execute in the background 
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include "apue.h"


/******************************************
 * 
 *      GLOBAL VARIABLES AND MACROS
 * 
 ******************************************/

#define MAX_CMDS 10
#define MAX_PIPES 10
#define MAX_FDS 10

char* notMyCmd[] = {"foo", "boo", "too", "matformatter", "matmult_t", NULL}; // Defines the special function
static void sig_cld(int);

/*
 * Single Linked List
 */
struct Node
{
	char *Data;
	struct Node *Next;
};


/******************************************
 * 
 *       PARSE COMMAND SUB-FUNCTIONS
 * 
 ******************************************/


/*
 * isNotMyCommand(): loop through the customized functions, if
 * it is non of the implemented cmd, return -1 else return 1
 */
int isNotMyCommand(char *cmd_arg[])
{
	int i = 0;
	while( notMyCmd[i] != NULL) {
		if ( (strstr(cmd_arg[0], notMyCmd[i])) != NULL ) {
			return -1;
		}
		i ++;
	}
	return 1;
}


/*
 * executeNotMyCommand(): execute command from Unix by calling execvp()
 */
void executeNotMyCommand(char *cmd_arg[])
{	
	execvp(cmd_arg[0], cmd_arg);
	
	fprintf(stderr, "Execvp failed with process %d on %s\n", getpid(), cmd_arg[0]);
	perror("Error:");
}


/*
 * executeMyCommand(): execute implemented command by execv()
 */
void executeMyCommand(char *cmd_arg[])
{	
	execv(cmd_arg[0], cmd_arg);
	
	fprintf(stderr, "Execv failed with process %d on %s\n", getpid(), cmd_arg[0]);
	perror("Error:");
}


/*
 * sighandler()- it handler the case when "ctrl-c" is pressed and all the 
 * "foreground" processes are stopped, with "myshell" still running
 */
void sighandler(int sig)
{
	(void) signal(SIGINT, SIG_DFL);
}


/*
 * sig_cld()- it print out the SIGCHLD when a backgound child process
 * terminates or finishes its process
 */
static void sig_cld(int signo)
{
	pid_t pid;
	int status;
	if ( pid < 0 ) {
		printf("SIGCHLD reveived\n");
	}
}


/******************************************
 * 
 *       PARSE COMMAND FUNCTIONS
 * 
 ******************************************/


/*
 * parseCommand(): given a command line, it parse the command line into token
 * separate it by space, and store all the arguments into a single linked list.
 * Next, it exectue one command at a time until the entire command line is
 * exhausted. Return 1 upon successfully exectued all commands. 
 *
 * Some or all of this code's structure was written by Prof. West's "shell.c".
 * And part of the structure was reference from Cornell University. 
 *
 */
int parseCommand(char *cmdLine)
{
	struct Node *Head;				// linked list to store cmdLine by token
	
	char *cmd, *tempCmd;			// string variable which store each cmd temporarly  
	char *token, *tempToken;		// string variable which used to store token returned by strtok()
	
	int cmdCount[MAX_CMDS];			// array of cmds
	int pid[MAX_PIPES];				// array of child process ID
	int fd[MAX_FDS][2];				// array of file descriptors
	
	int num_cmd = 0, num_pipe = 0;	// counter for number of pipe and cmd
	int firstData = -1;				// flag which indicates the first token
	int hasPipe = -1;				// flag which indicates cmdLine contains pipe
	int runInBackGround = -1;		// flag which indicates background job
	int i, j, k;
	
	// initial the cmdCount[] array
	for (i = 0; i < MAX_CMDS; i++) {
		cmdCount[i] = 0;
	}
	
	
	// parse in entire command line with spaces using strtok()
	// and store each toekn into a single liked list
	for (i = 0, cmd = cmdLine; ; i ++, cmd = NULL) {
		token = strtok(cmd, " ");
		if (token ==  NULL)
			break;	
		
		// trailing off the potential new line charactre on each token
		char *indexN = token;
		indexN = strstr(token, "\n");
		if (indexN) *indexN = '\0';
		
		
		// assign the data to a node 
		struct Node *temp1, *temp2;
		temp1 = (struct Node *) malloc(sizeof(struct Node));
		
		
		if ( strstr(token, "|") != NULL ) {	
			num_pipe ++;
			num_cmd ++;
		} else {
			
			if (token[strlen(token) - 1] == ';') {
				tempToken = malloc(strlen(token));
				
				for(j = 0; j < strlen(token) - 1; j++)
					tempToken[j] = token[j];
				
				tempToken[j] = '\0';
				
				temp1 -> Data = tempToken;
				
				cmdCount[num_cmd] ++;
				num_cmd ++;
				
			} else {
				temp1 -> Data = token;
				cmdCount[num_cmd] ++;
			}
			
			
			// add the data to the end of the linked list
			temp2 = Head;
			if (firstData < 0) {
				firstData = 1;
				Head = temp1;
				Head -> Next = NULL;
			}else {
				while(temp2 -> Next != NULL) {
					temp2 = temp2 -> Next;
				}
				
				temp1 -> Next = NULL;
				temp2 -> Next = temp1;
			}
		}
	} 
	
	// creat number of pipes according to the pipe count
	if (num_pipe > 0) {
		hasPipe = 1;
		for (k = 0; k < num_pipe; k++) {
			pipe(fd[k]);
		}
	}  
	
	
	struct Node *curPtr = Head;				// a currently pointer of linked list to the head
	
	// execute through one command at a time until the entire cmd line is exhausted
	for (i = 0; i <= num_cmd; i++) {
		
		char *envp[] = {NULL};				// array of enviornment variable
		
		char *cmd_arg[cmdCount[i]+1];		// store all arguments for each command into an array
		int num_arg = cmdCount[i];			// number of arguments for each command
		
		
		// string variable for different type of file name
		char *stdinFname, *stdoutFname;		
		char *stderrFname, *stdoutstderrFname;
		
		// flag to indetify each different type of I/O redirection 
		int hasStdinFile = -1, hasStdoutFile = -1;  
		int hasStderrFile = -1, hasStdoutStderr = -1;
		
		// fd for each different type of I/O
		int fdStdin, fdStdout, fdStderr;
		
		
		// read in all arguments in one command to raise 
		// any flag for special symbol
		for (j = 0, k = 0 ; j < num_arg; j++) {
			char *temp = curPtr -> Data;
			
			// condition to check any I/O redirection
			if (strstr(temp, "<") != NULL) {
				curPtr = curPtr -> Next;
				stdinFname = curPtr -> Data;
				hasStdinFile = 1; j++;
				curPtr = curPtr -> Next;
			} else if (strstr(temp, "1>") != NULL) {
				curPtr = curPtr -> Next;
				stdoutFname = curPtr -> Data;
				hasStdoutFile = 1; j++;
				curPtr = curPtr -> Next;
			} else if (strstr(temp, "2>") != NULL) {
				curPtr = curPtr -> Next;
				stderrFname = curPtr -> Data;
				hasStderrFile = 1; j++;
				curPtr = curPtr -> Next;
			} else if (strstr(temp, "&>") != NULL) {
				curPtr = curPtr -> Next;
				stdoutstderrFname = curPtr -> Data;
				hasStdoutStderr = 1; j++;
				curPtr = curPtr -> Next;
			} else if (strstr(temp, ">") != NULL) {
				curPtr = curPtr -> Next;
				stdoutFname = curPtr -> Data;
				hasStdoutFile = 1; j++;
				curPtr = curPtr -> Next;
			} else if (strstr(temp, "|") != NULL) {
				curPtr = curPtr -> Next; 
			} else if (strstr(temp, "&") != NULL) {
				runInBackGround = 1;
				curPtr = curPtr -> Next;
			} else {
				cmd_arg[j] = curPtr -> Data;
				curPtr = curPtr -> Next;
				k ++;
			}
		}
		
		
		// padding NULL to unused index in cmd_arg array
		for (; k <= num_arg; k++) {
			cmd_arg[k] = NULL;
		}
		
		// exectute the command
		if (cmd_arg[0] != NULL) {
			
			pid[i] = fork();
			
			// enter if it is a child
			if (pid[i] == 0){
				
				// rediret to stdin
				if (hasStdinFile > 0) {
					if ((fdStdin = open(stdinFname, O_RDONLY)) == -1) {
						fprintf(stderr, "can't open file %s\n", stdinFname);
						exit(1);
					}
					
					dup2(fdStdin, fileno(stdin));
					close(fdStdin);
				}
				
				
				// rediret to stdout
				if (hasStdoutFile > 0) {
					
					if ((fdStdout = open(stdoutFname, O_WRONLY | O_CREAT | O_TRUNC, 0600)) == -1) {
						fprintf(stderr, "can't create or write to file %s\n", stdoutFname);
						exit(2);
					}
					
					close(fileno(stdout));
					dup(fdStdout);
				}
				
				
				// redirect to stderr
				if (hasStderrFile > 0) {
					
					if ((fdStderr = open(stderrFname,O_WRONLY | O_CREAT | O_TRUNC, 0600)) == -1) {
						fprintf(stderr, "can't create or output erro file %s\n", stderrFname);
						exit(3);
					}
					
					dup2(fdStderr, fileno(stderr));					
				}
				
				
				// redirect to stdout and strderr
				if (hasStdoutStderr > 0) {
					
					if ((fdStdout = open(stdoutstderrFname, O_WRONLY | O_CREAT | O_TRUNC, 0600)) == -1) {
						fprintf(stderr, "can't create or write to file %s\n", stdoutstderrFname);
						exit(3);
					}
					
					close(fileno(stdout));
					dup(fdStdout);
					
					if ((fdStderr = open(stdoutstderrFname,O_WRONLY | O_CREAT | O_TRUNC, 0600)) == -1) {
						fprintf(stderr, "can't create or output erro file %s\n", stdoutstderrFname);
						exit(3);
					}
					
					dup2(fdStderr, fileno(stderr));
					close(fdStderr);
				}
				
				if (hasPipe > 0) {
					// first command
					if (i == 0) {
						// redirect stdout to pipe input
						if (num_pipe > 0) {
							
							if( fd[i][1] != STDOUT_FILENO) {
								dup2(fd[i][1], fileno(stdout));
							}
							close(fd[i][0]);
						}
						
						for (j = 0; j < num_pipe; j++) {
							if (j != i) {
								close(fd[j][0]);
								close(fd[j][1]);
							}
						}
						
						// last command
					} else if (i == num_cmd) {
						// redirect pipe output to stdin
						if (num_pipe > 0) {
							
							if( fd[i-1][0] != STDIN_FILENO) {
								dup2(fd[i-1][0], fileno(stdin));
							}
							close(fd[i-1][1]);
						}
						
						for (j = 0; j < num_pipe; j++) {
							if ( j != (i-1)) {
								close(fd[j][0]);
								close(fd[j][1]);
							}
						}
					} else {
						// redirect both stdout and stdin
						if (num_pipe > 0) {
							if( fd[i-1][0] != STDIN_FILENO) {
								dup2(fd[i-1][0], fileno(stdin));
							}
							close(fd[i-1][1]);
							
							if( fd[i][1] != STDOUT_FILENO) {
								dup2(fd[i][1], fileno(stdout));
							}
							close(fd[i][0]);
							
							// close all other unused pipe fds
							for (j = 0; j < num_pipe; j++) {
								if ( j != i && j != (i-1)) {
									close(fd[j][0]);
									close(fd[j][1]);
								}
							}
						}	
						
					}
				} // if for hasPipe
				
				if ( isNotMyCommand(cmd_arg) > 0) {
					executeNotMyCommand(cmd_arg);
				}
				else {
					executeMyCommand(cmd_arg);
				}		
				
				num_pipe --;
			
			} // if for pid[i] == 0
		} // if the cmd is valid
	} // for-loop for all cmds
	
	for (j = 0; j < num_pipe; j++) {
		close(fd[j][0]);
		close(fd[j][1]);
	}
	int status;
	if (runInBackGround < 0 ){
		for (j = 0; j <= num_cmd; j++) {
			waitpid(pid[j], &status, 0);
		}
	}
	
	if (runInBackGround > 0) {
		if (signal(SIGCHLD, sig_cld) == SIG_ERR)
			perror("signal error");
	}
	
	return 1;
}

/******************************************
 * 
 *            MAIN FUNCTION
 * 
 ******************************************/


/*
 * main(): prompt the users with "myshell >" and read in the entire line
 * of command at a time. It then calls the paserCommand() to execute
 * one command at a time. Upon termination of executing all commands, 
 * it prints another "myshee >" for a new command to input.
 *
 * The "myshell >" terminates automatically when an error occur or the user
 * press "ctrl+d" to kill the program.
 */
int main(int argc, char **argv) 
{
	FILE *fd;
	char line[LINE_MAX];
	int exitFlag;
	fd = stdin;
	do {
		
		(void) signal(SIGINT, sighandler);
		
		printf("myshell> ");
		
		fgets(line, LINE_MAX, fd);
		
		// terminates myshell when "ctrl-d" is pressed
		if (feof(fd) != 0) {
			kill(0, SIGKILL);
			return 0;
			
		// parse the command only if next empty
		} else if (strlen(line) > 2) 
			parseCommand(line);

		printf("\n");
		
	} while (1);
	
	return 0;
}

