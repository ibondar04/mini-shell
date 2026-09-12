#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include "builtins.h"



int handle_builtin(char * args[])
{
    if (strcmp(args[0], "exit") == 0)
    {
        return 2;
    }

    if (strcmp(args[0], "cd") == 0)
    {
        static char previous_dir[1024] = "";

        char old_cwd[1024];

        if (getcwd(old_cwd, sizeof(old_cwd)) == NULL)
        {
            perror("cd");
            return 1;

        }

        char *path = args[1];

        if (path == NULL)
        {
            path = getenv("HOME");
        }
        else if (strcmp(path, "-") == 0)
        {
            if (previous_dir[0] == '\0')
            {
                printf("cd: no previous directory\n");
                return 1;
            }

            path = previous_dir;
            printf("%s\n", path);
        }

        if (path == NULL)
        {
            printf("cd: HOME not set\n");
            return 1;
        }

        if (chdir(path) == -1)
        {
            perror("cd");
            return 1;
        }

        strcpy(previous_dir, old_cwd);
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