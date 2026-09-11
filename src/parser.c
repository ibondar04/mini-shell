#include <stdio.h>
#include <string.h>

#include "parser.h"



int parse_input(char *input, char *args[])
{
    int argc = 0;
    char *p = input;
    
    while (*p != '\0' && argc < MAX_ARGS - 1)
    {
        while (*p == ' ')
        {
            p++;
        }

        if (*p == '\0')
        {
            break;
        }

        if (*p == '"')
        {
            p++;

            args[argc] = p;
            argc++;

            while (*p != '\0' && *p != '"')
            {
                p++;
            }

            if (*p == '"')
            {
                *p = '\0';
                p++;
            }
        }
        else
        {
            args[argc] = p;
            argc++;

            while (*p != '\0' && *p != ' ')
            {
                p++;
            }

            if (*p == ' ')
            {
                *p = '\0';
                p++;
            }
        }
    }

    args[argc] = NULL;
    return argc;
}



int parse_redirection(char *args[], int arg_count, char **input_file, char **output_file)
{
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
    if (arg_count > 0 && strcmp(args[arg_count - 1], "&") == 0)
    {
        args[arg_count - 1] = NULL;
        return 1;
    }

    return 0;
}