#ifndef PARSER_H
#define PARSER_H


int parse_input(char *input, char *args[]);

int parse_redirection(char *args[], int arg_count, char **input_file, char **output_file);

int check_background(char *args[], int arg_count);


#endif