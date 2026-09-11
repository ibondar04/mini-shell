#ifndef EXECUTOR_H
#define EXECUTOR_H


void execute_command(char *args[], char *input_file, char *output_file, int background);

void execute_pipeline(char ***commands, int command_count, char *input_files[], char *output_files[]);


#endif