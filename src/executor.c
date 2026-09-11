#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>
#include <stdlib.h>

#include "executor.h"



void execute_command(char *args[], char *input_file, char *output_file, int background)
{
    pid_t pid = fork();

    if (pid < 0)
    {
        perror("fork");
        return;
    }

    if (pid == 0)
    {
        if (output_file != NULL)
        {
            int fd = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);

            if (fd == -1)
            {
                perror("open");
                exit(EXIT_FAILURE);
            }

            if (dup2(fd, STDOUT_FILENO) == -1)
            {
                perror("dup2");
                close(fd);
                exit(EXIT_FAILURE);
            }

            close(fd);
        }

        if (input_file != NULL)
        {
            int fd = open(input_file, O_RDONLY);

            if (fd == -1)
            {
                perror("open");
                exit(EXIT_FAILURE);
            }

            if (dup2(fd, STDIN_FILENO) == -1)
            {
                perror("dup2");
                close(fd);
                exit(EXIT_FAILURE);
            }

            close(fd);
        }

        signal(SIGINT, SIG_DFL);

        execvp(args[0], args);

        perror("execvp");
        exit(EXIT_FAILURE);
    }

    if (!background)
    {
        int status;

        if (waitpid(pid, &status, 0) == -1)
        {
            perror("waitpid");
        }
        else if (WIFSIGNALED(status) && WTERMSIG(status) == SIGINT)
        {
            printf("\n");
        }
    }
}



void execute_pipeline(char ***commands, int command_count, char *input_files[], char *output_files[])
{
    int pipefds[command_count - 1][2];

    for (int i = 0; i < command_count - 1; i++)
    {
        if (pipe(pipefds[i]) == -1)
        {
            perror("pipe");
            return;
        }
    }

    pid_t pids[command_count];

    for (int i = 0; i < command_count; i++)
    {
        pid_t pid = fork();

        if (pid < 0)
        {
            perror("fork");

            for (int j = 0; j < command_count - 1; j++)
            {
                close(pipefds[j][0]);
                close(pipefds[j][1]);
            }

            return;
        }

        pids[i] = pid;

        if (pid == 0)
        {
            if (i > 0)
            {
                if (dup2(pipefds[i - 1][0], STDIN_FILENO) == -1)
                {
                    perror("dup2");
                    exit(EXIT_FAILURE);
                }
            }

            if (i < command_count - 1)
            {
                if (dup2(pipefds[i][1], STDOUT_FILENO) == -1)
                {
                    perror("dup2");
                    exit(EXIT_FAILURE);
                }
            }

            if (input_files[i] != NULL)
            {
                int fd = open(input_files[i], O_RDONLY);

                if (fd == -1)
                {
                    perror("open");
                    exit(EXIT_FAILURE);
                }

                if (dup2(fd, STDIN_FILENO) == -1)
                {
                    perror("dup2");
                    close(fd);
                    exit(EXIT_FAILURE);
                }

                close(fd);
            }

            if (output_files[i] != NULL)
            {
                int fd = open(output_files[i], O_WRONLY | O_CREAT | O_TRUNC, 0644);

                if (fd == -1)
                {
                    perror("open");
                    exit(EXIT_FAILURE);
                }

                if (dup2(fd, STDOUT_FILENO) == -1)
                {
                    perror("dup2");
                    close(fd);
                    exit(EXIT_FAILURE);
                }

                close(fd);
            }

            for (int j = 0; j < command_count - 1; j++)
            {
                close(pipefds[j][0]);
                close(pipefds[j][1]);
            }

            signal(SIGINT, SIG_DFL);

            execvp(commands[i][0], commands[i]);

            perror("execvp");
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < command_count - 1; i++)
    {
        close(pipefds[i][0]);
        close(pipefds[i][1]);
    }

    int interrupted = 0;

    for (int i = 0; i < command_count; i++)
    {
        int status;

        if (waitpid(pids[i], &status, 0) == -1)
        {
            perror("waitpid");
        }
        else if (WIFSIGNALED(status) && WTERMSIG(status) == SIGINT)
        {
            interrupted = 1;
        }
    }

    if (interrupted)
    {
        printf("\n");
    }
}