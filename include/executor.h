#ifndef EXECUTOR_H
#define EXECUTOR_H


void execute_command(char *args[], char *input_file, char *output_file, int background);

void execute_pipe(char *args[], char *pipe_args[]);


#endif