#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

int main(void)
{
    char input[100];

    while (1)
    {
        printf("myshell>\n");
        fgets(input, sizeof(input), stdin);

        input[strcspn(input, "\n")] = '\0';

        if (strcmp(input, "exit") == 0)
        {
            break;
        }

        char *args[10];
        
        int i = 0;

        char *token = strtok(input, " ");

        while (token != NULL)
        {
            args[i] = token;
            i++;
            token = strtok(NULL, " ");
        }

        args[i] = NULL;

        if (strcmp(args[0], "exit") == 0)
        {
            break;
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

            continue;
        }

        pid_t pid = fork();

        if (pid == 0)
        {
            execvp(args[0], args);

            perror("execvp");
            return 1;
        }
        else
        {
            wait(NULL);
        }
    }
    
    return 0;
}