#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

int main(void)
{
    char input[100];

    while (1)
    {
        printf("myshell>\n");
        if (fgets(input, sizeof(input), stdin) == NULL)
        {
            break;
        }

        input[strcspn(input, "\n")] = '\0';

        char *args[10];
        
        int i = 0;

        char *token = strtok(input, " ");

        while (token != NULL && i < 9)
        {
            args[i] = token;
            i++;
            token = strtok(NULL, " ");
        }

        args[i] = NULL;

        if (args[0] == NULL)
        {
            continue;
        }

        char *output_file = NULL;
        char *input_file = NULL;

        int redirection_error = 0;

        for (int j = 0; j < i; j++)
        {
            if (strcmp(args[j], ">") == 0)
            {
                if (args[j + 1] == NULL)
                {
                    printf("myshell: missing output file\n");
                    redirection_error = 1;
                    break;
                }

                output_file = args[j + 1];
                args[j] = NULL;
                break;
            }

            if (strcmp(args[j], "<") == 0)
            {
                if (args[j + 1] == NULL)
                {
                    printf("myshell: missing input file\n");
                    redirection_error = 1;
                    break;
                }

                input_file = args[j + 1];
                args[j] = NULL;
                break;
            }
        }

        if (redirection_error)
        {
            continue;
        }

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

        if (pid < 0)
        {
            perror("fork");
        }
        else if (pid == 0)
        {
            if (output_file != NULL)
            {
                int fd = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);

                if (fd == -1)
                {
                    perror("open");
                    return 1;
                }

                if (dup2(fd, STDOUT_FILENO) == -1)
                {
                    perror("dup2");
                    close(fd);
                    return 1;
                }
                close(fd);
            }

            if (input_file != NULL)
            {
                int fd = open(input_file, O_RDONLY);

                if (fd == -1)
                {
                    perror("open");
                    return 1;
                }

                if (dup2(fd, STDIN_FILENO) == -1)
                {
                    perror("dup2");
                    close(fd);
                    return 1;
                }
                close(fd);
            }

            execvp(args[0], args);

            perror("execvp");
            return 1;
        }
        else
        {
            if (wait(NULL) == -1)
            {
                perror("wait");
            }
        }
    }
    
    return 0;
}