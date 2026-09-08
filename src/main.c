#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>

int main(void)
{
    char input[100];

    signal(SIGINT, SIG_IGN);

    while (1)
    {
        while (waitpid(-1, NULL, WNOHANG) > 0)
        {
        }

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

        int background = 0;

        if (i > 0 && strcmp(args[i - 1], "&") == 0)
        {
            background = 1;
            args[i - 1] = NULL;
        }

        char **pipe_args = NULL;

        for (int j = 0; j < i && args[j] != NULL; j++)
        {
            if (strcmp(args[j], "|") == 0)
            {
                args[j] = NULL;
                pipe_args = &args[j + 1];
                break;
            }
        }

        char *output_file = NULL;
        char *input_file = NULL;

        int redirection_error = 0;

        for (int j = 0; j < i && args[j] != NULL; j++)
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

        int pipefd[2];

        if (pipe_args != NULL)
        {
            if (pipe(pipefd) == -1)
            {
                perror("pipe");
                continue;
            }
        }

        if (pipe_args != NULL)
        {
            pid_t left_pid = fork();

            if (left_pid < 0)
            {
                perror("fork");
                close(pipefd[0]);
                close(pipefd[1]);
                continue;
            }

            if (left_pid == 0)
            {
                if (dup2(pipefd[1], STDOUT_FILENO) == -1)
                {
                    perror("dup2");
                    close(pipefd[0]);
                    close(pipefd[1]);
                    return 1;
                }
                
                close(pipefd[0]);
                close(pipefd[1]);

                signal(SIGINT, SIG_DFL);

                execvp(args[0], args);

                perror("execvp");
                return 1;
            }

            pid_t right_pid = fork();

            if (right_pid < 0)
            {
                perror("fork");
                close(pipefd[0]);
                close(pipefd[1]);
                wait(NULL);
                continue;
            }

            if (right_pid == 0)
            {
                if (dup2(pipefd[0], STDIN_FILENO) == -1)
                {
                    perror("dup2");
                    close(pipefd[0]);
                    close(pipefd[1]);
                    return 1;
                }

                close(pipefd[0]);
                close(pipefd[1]);

                signal(SIGINT, SIG_DFL);

                execvp(pipe_args[0], pipe_args);

                perror("execvp");
                return 1;
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

            signal(SIGINT, SIG_DFL);

            execvp(args[0], args);

            perror("execvp");
            return 1;
        }
        else
        {
            if (!background)
            {
                int status;

                if (wait(&status) == -1)
                {
                    perror("wait");
                }
                else if (WIFSIGNALED(status) && WTERMSIG(status) == SIGINT)
                {
                    printf("\n");
                }
            }
            
        }
    }
    
    return 0;
}