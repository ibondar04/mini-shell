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

        char *args[10];
        
        int i = parse_input(input, args);

        if (args[0] == NULL)
        {
            continue;
        }

        int background = check_background(args, i);

        char **pipe_args = NULL;

        for (int j = 0; j < i && args[j] != NULL; j++)
        {
            if (strcmp(args[j], "|") == 0)
            {
                args[j] = NULL;
                pipe_args = &args[j + 1];
                break;
            }
        }

        char *output_file = NULL;
        char *input_file = NULL;

        if (parse_redirection(args, i, &input_file, &output_file) == -1)
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

        if (pipe_args != NULL)
        {
            execute_pipe(args, pipe_args);
            continue;
        }

        execute_command(args, input_file, output_file, background);
    }
    
    return 0;
}