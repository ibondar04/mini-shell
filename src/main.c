#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <signal.h>

#include "parser.h"
#include "builtins.h"
#include "executor.h"



int main(void)
{
    char input[100];

    signal(SIGINT, SIG_IGN);

    while (1)
    {
        while (waitpid(-1, NULL, WNOHANG) > 0)
        {
        }

        printf("myshell>\n");
        if (fgets(input, sizeof(input), stdin) == NULL)
        {
            break;
        }

        input[strcspn(input, "\n")] = '\0';

        char *args[100];
        
        int i = parse_input(input, args);

        if (args[0] == NULL)
        {
            continue;
        }

        int background = check_background(args, i);

        char **commands[10];
        int command_count = 1;
        
        commands[0] = args;

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

                args[j] = NULL;
                commands[command_count] = &args[j + 1];
                command_count++; 
            }
        }

        if (command_count == 0)
        {
            continue;
        }

        char *output_files[10] = {NULL};
        char *input_files[10] = {NULL};

        for (int j = 0; j < command_count; j++)
        {
            int arg_count = 0;

            while (commands[j][arg_count] != NULL)
            {
                arg_count++;
            }
            
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

        

        int builtin_result = handle_builtin(args);

        if (builtin_result == 2)
        {
            break;
        }

        if (builtin_result == 1)
        {
            continue;
        }

        if (command_count > 1)
        {
            execute_pipeline(commands, command_count, input_files, output_files);
            continue;
        }

        execute_command(args, input_files[0], output_files[0], background);
    }
    
    return 0;
}