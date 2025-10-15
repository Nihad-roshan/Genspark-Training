#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAX_INPUT 1024
#define MAX_ARGS 64

int main()
{
    char input[MAX_INPUT];
    char *args[MAX_ARGS];

    while (1)
    {
        printf("mini-shell> "); // Prompt style
        fflush(stdout);

        // Read a line of input
        if (fgets(input, sizeof(input), stdin) == NULL)
            break; 

        // Remove newline character
        input[strcspn(input, "\n")] = '\0'; //replacing \n with \0

        
        if (strcmp(input, "exit") == 0) // for exit
            break;

        // Tokenize input into arguments
        int i = 0;
        char *token = strtok(input, " ");
        while (token != NULL && i < MAX_ARGS - 1)
        {
            args[i++] = token;
            token = strtok(NULL, " ");
        }
        args[i] = NULL; // Null-terminate the argument list

        if (args[0] == NULL)
            continue; //for  Empty command

        // Fork a child process
        pid_t pid = fork();

        if (pid < 0)
        {
            perror("fork failed");
            continue;
        }
        else if (pid == 0)
        {
            // In child: execute command
            execvp(args[0], args);
            perror("exec failed"); // Only runs if execvp fails
            exit(EXIT_FAILURE);
        }
        else
        {
            // In parent: wait for child
            int status;
            wait(&status);
            
            if (WIFEXITED(status))
            {//for showing the status given by wait
                printf("[Child exited with code %d]\n", WEXITSTATUS(status));
            }
        }
    }

    printf("Exiting mini-shell.\n");
    return 0;
}
