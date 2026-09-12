#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include "builtins.h"



BuiltinResult handle_builtin(char *args[])
{
    // Signal main() that the shell should terminate.
    if (strcmp(args[0], "exit") == 0)
    {
        return BUILTIN_EXIT;
    }

    if (strcmp(args[0], "cd") == 0)
    {
        // static keeps the previous directory between calls,
        // allowing "cd -" to return to the last directory.
        static char previous_dir[1024] = "";

        char old_cwd[1024];

        // Save the current directory before changing it.
        if (getcwd(old_cwd, sizeof(old_cwd)) == NULL)
        {
            perror("cd");
            return BUILTIN_HANDLED;
        }

        char *path = args[1];

        // "cd" with no argument goes to the user's home directory.
        if (path == NULL)
        {
            path = getenv("HOME");
        }
        // "cd -" switches back to the previous directory.
        else if (strcmp(path, "-") == 0)
        {
            if (previous_dir[0] == '\0')
            {
                printf("cd: no previous directory\n");
                return BUILTIN_HANDLED;
            }

            path = previous_dir;
            printf("%s\n", path);
        }

        if (path == NULL)
        {
            printf("cd: HOME not set\n");
            return BUILTIN_HANDLED;
        }

        if (chdir(path) == -1)
        {
            perror("cd");
            return BUILTIN_HANDLED;
        }

        // Only update the previous directory after chdir succeeds.
        strcpy(previous_dir, old_cwd);
        return BUILTIN_HANDLED;
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

        return BUILTIN_HANDLED;
    }

    // Not a built-in command.
    return BUILTIN_NOT_FOUND;
}