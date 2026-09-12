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
        // Apply output redirection before executing the command.
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

        // Apply input redirection before executing the command.
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

        // The shell ignores Ctrl+C, but foreground commands should not.
        signal(SIGINT, SIG_DFL);

        // Replace the child process with the requested program.
        execvp(args[0], args);

        // execvp() only returns if execution fails.
        perror("execvp");
        exit(EXIT_FAILURE);
    }

    if (background)
    {
        printf("[background] %d\n", pid);
    }

    // Foreground commands block the shell until they finish.
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



void execute_pipeline(char ***commands, int command_count, char *input_files[], char *output_files[], int background)
{
    // A pipeline with N commands requires N - 1 pipes.
    int pipefds[command_count - 1][2];

    for (int i = 0; i < command_count - 1; i++)
    {
        if (pipe(pipefds[i]) == -1)
        {
            perror("pipe");

            // Close any pipes that were successfully created earlier.
            for (int j = 0; j < i; j++)
            {
                close(pipefds[j][0]);
                close(pipefds[j][1]);
            }
            
            return;
        }
    }

    // Store each child's PID so the parent can wait for them later.
    pid_t pids[command_count];

    for (int i = 0; i < command_count; i++)
    {
        pid_t pid = fork();

        if (pid < 0)
        {
            perror("fork");

            // Close all pipe descriptors in the parent.
            for (int j = 0; j < command_count - 1; j++)
            {
                close(pipefds[j][0]);
                close(pipefds[j][1]);
            }

            // Wait for any children that were already created.
            for (int j = 0; j < i; j++)
            {
                waitpid(pids[j], NULL, 0);
            }

            return;
        }

        pids[i] = pid;

        if (pid == 0)
        {
            // Every command except the first reads from the previous pipe.
            if (i > 0)
            {
                if (dup2(pipefds[i - 1][0], STDIN_FILENO) == -1)
                {
                    perror("dup2");
                    exit(EXIT_FAILURE);
                }
            }

            // Every command except the last writes to the next pipe.
            if (i < command_count - 1)
            {
                if (dup2(pipefds[i][1], STDOUT_FILENO) == -1)
                {
                    perror("dup2");
                    exit(EXIT_FAILURE);
                }
            }

            // Explicit input redirection overrides input from a pipe.
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

            // Explicit output redirection overrides output to a pipe.
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

            // After dup2(), the child no longer needs the original pipe descriptors.
            for (int j = 0; j < command_count - 1; j++)
            {
                close(pipefds[j][0]);
                close(pipefds[j][1]);
            }

            signal(SIGINT, SIG_DFL);

            // Replace this child with its command from the pipeline.
            execvp(commands[i][0], commands[i]);

            perror("execvp");
            exit(EXIT_FAILURE);
        }
    }

    // The parent does not read from or write to the pipes.
    // Leaving these open could prevent commands from receiving EOF.
    for (int i = 0; i < command_count - 1; i++)
    {
        close(pipefds[i][0]);
        close(pipefds[i][1]);
    }

    if (background)
    {
        printf("[background]");

        for (int i = 0; i < command_count; i++)
        {
            printf(" %d", pids[i]);
        }

        printf("\n");
    }

    if (!background)
    {
        int interrupted = 0;

        // Wait for every command in the foreground pipeline.
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
}