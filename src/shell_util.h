#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

#define MAX_INPUT_LENGTH 255
#define MAX_COMMANDS 100

/*
 *  'shell_util.h' includes function declarations for:
 *      - prompt
 *      - error_message
 *      - builtin_command
 *      - external_command
 */

void prompt();
void error_message();
void builtin_command(char* command, char* path[], int* path_count, char* words[], int words_counter);
int external_command(char* command, char* path[], int path_counter, char* words[], int words_counter);