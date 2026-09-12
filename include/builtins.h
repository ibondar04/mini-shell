#ifndef BUILTINS_H
#define BUILTINS_H


typedef enum
{
    BUILTIN_NOT_FOUND,
    BUILTIN_HANDLED,
    BUILTIN_EXIT
} BuiltinResult;

BuiltinResult handle_builtin(char *args[]);


#endif