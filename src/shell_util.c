#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

// include header file for shell_util.c
#include "shell_util.h"

/*
 *  'shell_util.c' includes function definitions for:
 *      - prompt()
 *      - error_message()
 *      - builtin_command()
 *      - external_command()
 */

/* The prompt function prints the RUSH prompt: "rush> " and flushes the output stream, 
 * immediately displaying the prompt. */
void prompt() {
    printf("rush> ");
    fflush(stdout);
}

/* The error_message function prints the program's standard error message and flushes
 * stderr. */
void error_message() {
    char error_message[30] = "An error has occurred\n";
    write(STDERR_FILENO, error_message, strlen(error_message));
    fflush(stderr);
}

/* The builtin_command function executes built-in shell commands, such as path, cd, and
 * exit. These commands are handled by the shell. */
void builtin_command(char* command, char* path[], int *path_count, char* words[], int words_counter) {
    // execute commands based on words[0]
    
    // path command
    if (strcmp(words[0], "path") == 0) {
        for (int i = 0; i < *path_count; ++i) {
            path[i] = NULL;
        }
        *path_count = 0;

        // if path is specified, update path array and path_count
        if (words_counter > 1) {
            for (int i = 1; i < words_counter; i++) {
                path[i - 1] = strdup(words[i]); // copy new paths
                (*path_count)++;
            }
        }
        path[*path_count] = NULL;
    // cd command
    } else if (strcmp(words[0], "cd") == 0) {
        // cd takes exactly 1 argument
        if (words_counter != 2) {
            error_message();
        } else {
            // working change directories
            int cd_arg = chdir(words[1]);
            if (cd_arg != 0) { // chdir() returns 0 if success; 1 if fail
                error_message();
            }
        }
    // exit command
    } else if (strcmp(words[0], "exit") == 0) {
        // error to pass any args to exit
        if (words_counter > 1) {
            error_message();
        } else {
            exit(0);
        }
    }
}

/* The function external_command deals with external commands. To execute an external command, it
 * forks a new process. This function searches for the specified command and executes it using execv. If
 * the command includes the redirection symbol ">", it redirects the child process' standard output to the
 * specified file. The function returns the process ID of the forked child process if successful. */
int external_command(char* command, char* path[], int path_counter, char* words[], int words_counter) {
    int rc = fork();
    if (rc < 0) { // fork failed
        error_message();
    } else if (rc == 0) { // child process
        // child: redirect standard output to a file
        char* output_filename = NULL;
        int redirect_found = 0;

        // find if there is redirection
        for (int i = 0; words[i] != NULL; i++) {
            if (strcmp(words[i], ">") == 0) {
                // check if ">" is first token
                if (words[i] == words[0]) {
                    error_message();
                    exit(1);
                }
                
                // if redirection symbol already found
                if (redirect_found) {
                    error_message();
                    exit(1);
                }
                redirect_found = 1;

                // check if there's an output filename provided and no additional arguments after the filename
                if (words[i + 1] != NULL && words[i + 2] == NULL) {
                    output_filename = words[i + 1];
                    words[i] = NULL;
                } else {
                    // no output file is specified or there are 1+ tokens after file name
                    error_message();
                    exit(1);
                }
            }
        }

        // perform redirection if needed
        if (redirect_found && output_filename != NULL) {
            int output_fd = open(output_filename, O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR);
            if (output_fd < 0) {
                error_message();
                exit(1);
            }
            if (dup2(output_fd, STDOUT_FILENO) < 0) { // redirect STDOUT to the output file
                error_message();
                exit(1);
            }
            close(output_fd); // close the file
        }

        // path
        for (int i = 0; path[i] != NULL; i++) {
            char full_path[MAX_INPUT_LENGTH + 1];
            snprintf(full_path, sizeof(full_path), "%s/%s", path[i], command);

            // check if the executable exists in this directory
            if (access(full_path, X_OK) == 0) {
                execv(full_path, words);
                // if execv returns, there is an error
                error_message();
            }
        }

        error_message(); // command wasn't found
        exit(0);
    } 
    return rc;
}