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



void execute_pipe(char *args[], char *pipe_args[])
{
    int pipefd[2];

    if (pipe(pipefd) == -1)
    {
        perror("pipe");
        return;
    }

    pid_t left_pid = fork();

    if (left_pid < 0)
    {
        perror("fork");
        close(pipefd[0]);
        close(pipefd[1]);
        return;
    }

    if (left_pid == 0)
    {
        if (dup2(pipefd[1], STDOUT_FILENO) == -1)
        {
            perror("dup2");
            close(pipefd[0]);
            close(pipefd[1]);
            exit(EXIT_FAILURE);
        }

        close(pipefd[0]);
        close(pipefd[1]);

        signal(SIGINT, SIG_DFL);

        execvp(args[0], args);

        perror("execvp");
        exit(EXIT_FAILURE);
    }

    pid_t right_pid = fork();

    if (right_pid < 0)
    {
        perror("fork");

        close(pipefd[0]);
        close(pipefd[1]);

        waitpid(left_pid, NULL, 0);
        return;
    }

    if (right_pid == 0)
    {
        if (dup2(pipefd[0], STDIN_FILENO) == -1)
        {
            perror("dup2");
            close(pipefd[0]);
            close(pipefd[1]);
            exit(EXIT_FAILURE);
        }

        close(pipefd[0]);
        close(pipefd[1]);

        signal(SIGINT, SIG_DFL);

        execvp(pipe_args[0], pipe_args);

        perror("execvp");
        exit(EXIT_FAILURE);
    }

    close(pipefd[0]);
    close(pipefd[1]);

    int left_status;
    int right_status;

    waitpid(left_pid, &left_status, 0);
    waitpid(right_pid, &right_status, 0);

    if ((WIFSIGNALED(left_status) && WTERMSIG(left_status) == SIGINT) ||
        (WIFSIGNALED(right_status) && WTERMSIG(right_status) == SIGINT))
    {
        printf("\n");
    }
}