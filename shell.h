// This is the header file for the mini-shell program. 
// The functions and typedefs are mainly 5 parts as following.

// =======================================================
//               Command execution functions
// =======================================================

// Check and run a built-in command.
// Check the user input, if it matches a built-in function,
// run the corresponding function and return 1 (if not exit);
// otherwise, return 0 (then try to run as other commands)
int runBuiltInCommand(char** argv);

// When user input does not match built-in functions, run the command
// by creating a child process. If command is not found in system,
// just print out a friendly reminder.
void runOtherCommand(char** argv);

// When user input contains one '|', create a child process to deal with
// the two commands. Note: this child process would fork again to execute
// the two commands (in function "pipeOperation").
void runPipeCommands(char** argvBeforePipe, char** argvAfterPipe);

// Run two pipe commands by forking to two processes. Let the output of the
// before-pipe process be the input of the after-pipe process.
void pipeOperation(char** argvBeforePipe, char** argvAfterPipe);

// Run a before-pipe or after-pipe command. If it matches a built-in function,
// execute the corresponding function and exit the process with status code 0;
// otherwise, run as a system command. If command is also not found in system,
// exit the process with status code 1 (trigger a friendly reminder in its
// parent process).
void runAPipeCommand(char** argv);


// =======================================================
//                   Built-in functions
// =======================================================

// Response to 'cd' command, change working directories.
// Only the first argument after "cd" is considered.
void cdMiniShell(char** argv);

// Response to 'echo' command, print out user input after echo command.
void echoMiniShell(char** argv);

// Response to 'exit' command, terminate the shell.
// Arguments after "exit" command would be ignored, except those after '|'
void exitMiniShell(char** argv);

// Response to 'help' command, print out message explaining
// all built-in commands.
// Arguments after "exit" command would be ignored, except those after '|'
void helpMiniShell(char** argv);

// Response to 'history' command, print out user input history.
// Arguments after "history" command would be ignored, except those after '|'
void historyMiniShell(char** argv);


// =======================================================
//                     Signal handler
// =======================================================

// Enable user to terminate the shell by pressing Ctrl+C.
void sigint_handler(int sig);


// =======================================================
//                User input pre-processing
// =======================================================

// Check whether user input characters are all ' 'or'\n'
// return 1 if yes, otherwise return 0.
int isSpaceString(char* input);

// Split user input into separate tokens, which would be
// function arguments.
void parseTokens(char** argv, char* input);


// ========================================================
// User input history handling -- LinkedList data structure
// ========================================================

// A node of the linkedList, has data and next node pointer.
typedef struct node {
    char* data;           // string type, stores the user input
    struct node* next;
} node_t;

// The linkedList, has the head node pointer of the list.
typedef struct linkedlist {
    node_t* head;
} linkedlist_t;

// Create an empty list, with head initialized to NULL.
linkedlist_t* createList();

// Update history list by appending the most recently user input
// to the end of the list.
void updateHistoryList(char* input);

// Free the memory allocated for the list, including the node data,
// the nodes, and the list pointer.
void freeHistoryList();
