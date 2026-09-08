#include <stdio.h>
#include <string.h>

#include "parser.h"



int parse_input(char *input, char *args[])
{
    int i = 0;
    
    char *token = strtok(input, " ");
    
    while (token != NULL && i < 9)
    {
        args[i] = token;
        i++;
        token = strtok(NULL, " ");
    }

    args[i] = NULL;

    return i;
}



int parse_redirection(char *args[], int arg_count, char **input_file, char **output_file)
{
    for (int j = 0; j < arg_count && args[j] != NULL; j++)
    {
        if (strcmp(args[j], ">") == 0)
        {
            if (args[j + 1] == NULL)
            {
                printf("myshell: missing output file\n");
                return -1;
            }

            *output_file = args[j + 1];
            args[j] = NULL;
            break;
        }

        if (strcmp(args[j], "<") == 0)
        {
            if (args[j + 1] == NULL)
            {
                printf("myshell: missing input file\n");
                return -1;
            }

            *input_file = args[j + 1];
            args[j] = NULL;
            break;
        }
    }

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