#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <signal.h>
#include <unistd.h>

#include "parser.h"
#include "builtins.h"
#include "executor.h"



int main(void)
{
    char input[100];

    // keep Ctrl+C from terminating the shell itself.
    // Child processes restore the default SIGINT behavior before execution.

    signal(SIGINT, SIG_IGN);

    while (1)
    {
        pid_t finished_pid;

        // Reap finished background processes without blocking the shell.
        while ((finished_pid = waitpid(-1, NULL, WNOHANG)) > 0)
        {
            printf("[background finished] %d\n", finished_pid);
        }

        // Display the current working directory as part of the shell prompt.
        char cwd[1024];

        if (getcwd(cwd, sizeof(cwd)) != NULL)
        {
            printf("myshell:%s$ ", cwd);
        }
        else
        {
            printf("myshell$ ");
        }
        
        if (fgets(input, sizeof(input), stdin) == NULL)
        {
            break;
        }

        input[strcspn(input, "\n")] = '\0';

        char *args[MAX_ARGS];
        
        int i = parse_input(input, args);

        // A negative result means the parser found invalid input,
        // such as an unmatched quote.
        if (i == -1)
        {
            continue;
        }

        if (args[0] == NULL)
        {
            continue;
        }

        // Remove a trailing '&' and remember whether the command
        // should run in the background.
        int background = check_background(args, i);

        if (background)
        {
            i--;
        }

        // Each element points to the beginning of one command
        // in a pipeline.
        char **commands[MAX_COMMANDS];
        int command_count = 1;
        
        commands[0] = args;

        // Split the argument array at each pipe.
        // Replacing "|" with NULL creates separate argv arrays
        // that can later be passed directly to execvp().
        for (int j = 0; j < i; j++)
        {
            if (strcmp(args[j], "|") == 0)
            {
                if (j == 0 || args[j - 1] == NULL)
                {
                    printf("myshell: missing command before pipe\n");
                    command_count = 0;
                    break;
                }

                if (j + 1 >= i || args[j + 1] == NULL)
                {
                    printf("myshell: missing command after pipe\n");
                    command_count = 0;
                    break;
                }

                if (command_count >= MAX_COMMANDS)
                {
                    printf("myshell: too many commands in pipeline\n");
                    command_count = 0;
                    break;
                }

                args[j] = NULL;
                commands[command_count] = &args[j + 1];
                command_count++; 
            }
        }

        if (command_count == 0)
        {
            continue;
        }

        // Store redirection information separately for each
        // command in the pipeline.
        char *output_files[MAX_COMMANDS] = {NULL};
        char *input_files[MAX_COMMANDS] = {NULL};

        for (int j = 0; j < command_count; j++)
        {
            int arg_count = 0;

            while (commands[j][arg_count] != NULL)
            {
                arg_count++;
            }
            
            // parse_redirection removes < and > tokens from the
            // command and stores the associated filenames.
            if (parse_redirection(commands[j], arg_count, &input_files[j], &output_files[j]) == -1)
            {
                command_count = 0;
                break;
            }
        }

        if (command_count == 0)
        {
            continue;
        }

        // Built-ins must be handled by the shell process itself.
        BuiltinResult  builtin_result = handle_builtin(args);

        if (builtin_result == BUILTIN_EXIT)
        {
            break;
        }

        if (builtin_result == BUILTIN_HANDLED)
        {
            continue;
        }

        if (command_count > 1)
        {
            execute_pipeline(commands, command_count, input_files, output_files, background);
            continue;
        }

        execute_command(args, input_files[0], output_files[0], background);
    }
    
    return 0;
}