#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "builtins.h"



int handle_builtin(char * args[])
{
    if (strcmp(args[0], "exit") == 0)
    {
        return 2;
    }

    if (strcmp(args[0], "cd") == 0)
    {
        if (args[1] == NULL)
        {
            printf("cd: missing argument\n");
        }
        else if (chdir(args[1]) != 0)
        {
            perror("cd");
        }

        return 1;
    }

    if (strcmp(args[0], "pwd") == 0)
    {
        char cwd[1024];

        if (getcwd(cwd, sizeof(cwd)) == NULL)
        {
            perror("pwd");
        }
        else
        {
            printf("%s\n", cwd);
        }

        return 1;
    }

    return 0;
}