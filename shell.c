#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <ctype.h>
#include "shell.h"        // the header file for the program (with docs)

#define MAX_BUFFER_SIZE 80
#define MAX_ARGUMENTS_NUMBER 16   // assume at most 16 arguments in a command

static linkedlist_t* historyList;    // The history list pointer

int main() {
	alarm(180);
	signal(SIGINT, sigint_handler);
	historyList = createList();
	while (1) {                  // exit by "exit" command or Ctrl+C
		write(1, "mini-shell>>> ", 15);
		char line[MAX_BUFFER_SIZE];
		fgets(line, MAX_BUFFER_SIZE, stdin);
		updateHistoryList(line);
		int i = isSpaceString(line);  // check whether all whitespaces									
		if (i == 1) {
			continue;              // do not respond to these commands
		}
		else {             // execute based on whether there is a pipe
			char* pipeChar = strchr(line, '|');
			if (pipeChar == NULL) {         // no pipe '|' in commands
				char* myargv[MAX_ARGUMENTS_NUMBER];
				parseTokens(myargv, line);
				if (runBuiltInCommand(myargv) == 0) {  // not built-in
					runOtherCommand(myargv);
				}
			} 
			else {                          // has pipe '|' in commands
				*pipeChar = '\0';	 // split the arguments at the pipe '|'
				char* argvBeforePipe[MAX_ARGUMENTS_NUMBER];
				char* argvAfterPipe[MAX_ARGUMENTS_NUMBER];
				parseTokens(argvBeforePipe, line);
				parseTokens(argvAfterPipe, pipeChar+1);
				runPipeCommands(argvBeforePipe, argvAfterPipe);
			}	
		}
	}
	return 0;
}

// >>>>>>>>>> Command execution functions <<<<<<<<<<<

int runBuiltInCommand(char** argv) {
	const char* commandNames[] = {"cd", "echo", "exit", "help", "history"};
	void (*functionPointers[]) (char**) = {cdMiniShell, echoMiniShell, 
					  exitMiniShell, helpMiniShell, historyMiniShell};
	int i = 0;
	while (i < 5) {
		if (strcmp(argv[0], commandNames[i]) == 0) {
			functionPointers[i](argv);
			return 1;      // built-in command executed successfully
		}
		i++;
	}
	return 0;              // not a built-in command
}

void runOtherCommand(char** argv) {
	pid_t childProcessID;
	int childStatus;
	childProcessID = fork();
	if (-1 == childProcessID) {
		puts("fork failed for some reason!");
		exit(EXIT_FAILURE);
	}
	if (childProcessID == 0) {
		execvp(argv[0], argv);
		exit(1);         // command not found
	}
	else {
		waitpid(childProcessID, &childStatus, 0);
		if (WEXITSTATUS(childStatus) == 1) {      // commmand not found
			puts("Command not found--Did you mean something else?");
		}
	}
}

void runPipeCommands(char** argvBeforePipe, char** argvAfterPipe) {
	pid_t childProcessID;
	int childStatus;
	childProcessID = fork();
	if (-1 == childProcessID) {
		puts("fork failed for some reason!");
		exit(EXIT_FAILURE);
	}
	if (childProcessID == 0) {
		pipeOperation(argvBeforePipe, argvAfterPipe);
	}
	else {
		waitpid(childProcessID, &childStatus, 0);
		if (WEXITSTATUS(childStatus) == 1) {   // command after '|' not found
			puts("Command not found--Did you mean something else?");
		}
	}
}

void pipeOperation(char** argvBeforePipe, char** argvAfterPipe) {
	int fd[2];
	pipe(fd);
	int childStatus;
	pid_t childProcessID;
	childProcessID = fork();
	if (-1 == childProcessID) {
		printf("fork failed for some reason!");
		exit(EXIT_FAILURE);
	}
	if (childProcessID == 0) {
		close(STDOUT_FILENO);
		dup2(fd[1], STDOUT_FILENO);
		close(fd[1]);
		close(fd[0]);
		runAPipeCommand(argvBeforePipe);
	}
	else {
		waitpid(childProcessID, &childStatus, 0);	
		if (WEXITSTATUS(childStatus) == 1) {  // command before '|' not found
			puts("Command not found--Did you mean something else?");
		}
		close(STDIN_FILENO);
		dup2(fd[0], 0);
		close(fd[1]);
		runAPipeCommand(argvAfterPipe);
	}
}

void runAPipeCommand(char** argv) {
	if (runBuiltInCommand(argv) == 1) {
		exit(0);         // built-in command executed successfully
	}					 // need to exit the process
	else {
		execvp(argv[0], argv);
		exit(1);         // command not found
	}
}

// >>>>>>>>>> Built-in functions <<<<<<<<<<<

// If no directory is given, do not respond.
// If given directory is invalid, print reminder message.
// Only the first argument after "cd" is considered.
void cdMiniShell(char** argv) {
	if (argv[1] != NULL && chdir(argv[1]) != 0) {
		printf("cd: %s: No such file or directory!\n", argv[1]);
	}
}

void echoMiniShell(char** argv) {
	int i = 1;
	while (argv[i] != NULL) {
		printf("%s ", argv[i++]);
	}
	printf("\n");
}

void exitMiniShell(char** argv) {
	freeHistoryList();
	exit(0);           // exit the current process
}

void helpMiniShell(char** argv) {
	printf("Mini-shell program developed by Fan Yang.\n");
	printf("Type a program name and arguments.\n");
	printf("Built-in commands:\n");
	printf("\tcd -- change working directories\n");
	printf("\techo -- print out user input after echo command\n");
	printf("\texit -- terminate the shell\n");
	printf("\thelp -- print out message explaining all built-in commands\n");
	printf("\thistory -- print out user input history\n");
}

void historyMiniShell(char** argv) {
	if (historyList == NULL || historyList->head == NULL) {
		return;
	}
	node_t* iter = historyList->head;
	int i = 1;
	while (iter != NULL) {
		printf("%4d    %s", i++, iter->data);
		iter = iter->next;
	}
}

// >>>>>>>>>> Signal handler <<<<<<<<<<<

void sigint_handler(int sig) {
	write(1, "mini-shell terminated!\n", 24);
	freeHistoryList();
	exit(0);
}

// >>>>>>>>>> User input pre-processing <<<<<<<<<<<

int isSpaceString(char* input) {
	int i = 0;
	while (input[i] != '\0') {
		if (isspace(input[i]) == 0) {
			return 0;
		}
		i++;
	}
	return 1;
}

void parseTokens(char** argv, char* input) {
	char* token;
	const char* delim = " \n";     // ' ' and '\n'
	token = strtok(input, delim);
	int i = 0;
	while (token != NULL) {
		argv[i++] = token;
		token = strtok(NULL, delim);
	}
	argv[i] = NULL;    // add NULL as the last argument
}

// >>>>>>>>>> User input history handling <<<<<<<<<<<

linkedlist_t* createList() {
	linkedlist_t* list = (linkedlist_t*)malloc(sizeof(linkedlist_t) * 1);
	list->head = NULL;
	return list;
}

void updateHistoryList(char* input) {
	if (historyList == NULL) {
		return;
	}
	node_t* newHistory = (node_t*)malloc(sizeof(node_t) * 1);
	newHistory->data = (char*)malloc(sizeof(char) * (strlen(input) + 1));
	strcpy(newHistory->data, input);
	newHistory->next = NULL;
	if (historyList->head == NULL) {
		historyList->head = newHistory;
	}
	else {
		node_t* iter = historyList->head;
		while (iter->next != NULL) {
			iter = iter->next;
		}
		iter->next = newHistory;
	}
}

void freeHistoryList() {
	if (historyList == NULL) {
		return;   
	}
	node_t* currentIter = historyList->head;
	while(currentIter != NULL) {
		node_t* nextIter = currentIter->next;
		free(currentIter->data);
		free(currentIter);
		currentIter = nextIter;
	}
	free(historyList);
}
