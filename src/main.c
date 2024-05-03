#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

// header for utility functions
#include "shell_util.h"

/*
 *  RUSH Shell:
 *  'main.c' handles reading input and executing commands in the interactive loop.
 */

int main(int argc, char* argv[]) {
    // shell must be invoked with NO arguments
    if (argc > 1) {
        error_message();
        exit(1);
    }

    // buffer for reading input
    size_t buffer_size = MAX_INPUT_LENGTH + 1; // 255 chars + 1 null terminator
    char *buffer = (char *)malloc(buffer_size * sizeof(char));
    if (buffer == NULL) {
        perror("Error. Unable to allocate buffer.");
        exit(1);
    }

    // initialize path
    int path_count = 0;
    char *path[MAX_INPUT_LENGTH + 1] = {"/bin", "/usr/bin", NULL};
    path[path_count] = "/bin";

    // RUSH: interactive loop
    while(1) {
        // repeatedly calls prompt, which prints "rush> "
        prompt();

        // read line of input
        size_t string = getline(&buffer, &buffer_size, stdin);

        if (string == -1) {
            // eof detection
            if (feof(stdin)) break;
            else {
            // error reading line
                error_message();
                continue;
            }
        }

        // remove \n from input line
        if (strlen(buffer) > 0 && buffer[strlen(buffer) - 1] == '\n')
            buffer[strlen(buffer) - 1] = '\0';

        char *commands[MAX_COMMANDS];
        int num_commands = 0; // counter for num of commands
        char *next_command;
        char *command_str = buffer; // mutable copy of buffer for strsep

        // parse input line into separate commands based on '&'
        while ((next_command = strsep(&command_str, "&")) != NULL && num_commands < MAX_COMMANDS) {
            commands[num_commands++] = next_command;
        }

        // execute each command
        pid_t* pids = (pid_t*)malloc(num_commands * sizeof(pid_t));
        if (pids == NULL) {
            // memory allocation failure
            perror("Failed to allocate memory for pids array");
            exit(1);
        }

        // initialize pids array to -1
        for (int i = 0; i < num_commands; i++) {
            pids[i] = -1; // initialize to -1: it is unused or a built-in command
        }

        // loop through each command
        for (int i = 0; i < num_commands; i++) {
            char *command = commands[i];

            char *words[MAX_INPUT_LENGTH + 1];
            int words_counter = 0;

            // split each command into words
            char *token;
            char *delimiter = " ";

            while ((token = strsep(&command, delimiter)) != NULL) {
                if (strlen(token) == 0) continue; // skip empty tokens
                words[words_counter++] = token;
            }

            words[words_counter] = NULL; // null-terminate list of words

            // skip empty commands
            if (words_counter == 0) continue;
            
            // commands:
            if (words_counter > 0) {
                // built-in commands
                if ((strcmp(words[0], "path") == 0) || (strcmp(words[0], "cd") == 0) || (strcmp(words[0], "exit") == 0)) {
                    builtin_command(words[0], path, &path_count, words, words_counter);
                // external commands
                } else {
                    int return_pid = external_command(words[0], path, path_count, words, words_counter);
                    if (return_pid > 0) { // successful fork
                        pids[i] = return_pid;
                    }
                }
            }
        }

        // wait for all child processes to complete
        for (int i = 0; i < num_commands; i++) {
            if (pids[i] > 0)
                waitpid(pids[i], NULL, 0);
        }
    } 
    // free buffer
    free(buffer);
    return 0;
}