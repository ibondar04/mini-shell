#include <stdio.h>
#include <string.h>

#include "parser.h"



int parse_input(char *input, char *args[])
{
    int argc = 0;

    // p reads through the original input while write rebuilds
    // each argument in place without quote/escape characters.
    char *p = input;
    char quote = '\0';
    
    while (*p != '\0')
    {
        // Skip spaces between arguments.
        while (*p == ' ')
        {
            p++;
        }

        if (*p == '\0')
        {
            break;
        }

        if (argc >= MAX_ARGS - 1)
        {
            printf("myshell: too many arguments\n");
            return -1;
        }   

        // Mark the start of the next argument.
        args[argc] = p;
        argc++;

        char *write = p;
        quote = '\0';

        while (*p != '\0')
        {
            // Handle basic escapes such as \" , \' , \\ , and escaped spaces.
            // Other backslashes are preserved for the program itself.
            if (*p == '\\' && *(p + 1) != '\0')
            {
                char next = *(p + 1);

                if (next == '"' || next == '\'' || next == '\\' || next == ' ')
                {
                    p++;

                    *write = *p;
                    write++;
                    p++;

                    continue;
                }
            }

            // Outside quotes, spaces end the current argument.
            if (quote == '\0')
            {
                if (*p == ' ')
                {
                    break;
                }

                // Enter single- or double-quote mode.
                if (*p == '"' || *p == '\'')
                {
                    quote = *p;
                    p++;
                    continue;
                }
            }
            else
            {
                // Leave quote mode when the matching quote is found.
                if (*p == quote)
                {
                    quote = '\0';
                    p++;
                    continue;
                }
            }

            // Copy normal characters into the cleaned argument.
            *write = *p;
            write++;
            p++;
        }

        if (quote != '\0')
        {
            printf("myshell: unmatched quote\n");
            return -1;
        }

        if (*p == ' ')
        {
            p++;
        }

        // Terminate the cleaned argument as a C string.
        *write = '\0';
    }

    // execvp() requires the argument array to end with NULL.
    args[argc] = NULL;

    return argc;
}



int parse_redirection(char *args[], int arg_count, char **input_file, char **output_file)
{
    // Compact the argument array while removing <, >,
    // and their associated filenames.
    int write_index = 0;

    for (int j = 0; j < arg_count && args[j] != NULL; j++)
    {
        if (strcmp(args[j], ">") == 0)
        {
            if (j + 1 >= arg_count || args[j + 1] == NULL)
            {
                printf("myshell: missing output file\n");
                return -1;
            }

            *output_file = args[j + 1];

            // Skip the filename because it should not be passed to execvp().
            j++;
        }
        else if (strcmp(args[j], "<") == 0)
        {
            if (j + 1 >= arg_count || args[j + 1] == NULL)
            {
                printf("myshell: missing input file\n");
                return -1;
            }

            *input_file = args[j + 1];

            // Skip the filename because it should not be passed to execvp().
            j++;
        }
        else
        {
            args[write_index] = args[j];
            write_index++;
        }
    }

    args[write_index] = NULL;

    return 0;
}



int check_background(char *args[], int arg_count)
{
    // '&' is treated as background syntax only when it is
    // the final argument of the command line.
    if (arg_count > 0 && strcmp(args[arg_count - 1], "&") == 0)
    {
        args[arg_count - 1] = NULL;
        return 1;
    }

    return 0;
}