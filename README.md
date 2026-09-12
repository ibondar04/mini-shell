# Mini Shell

A Unix-like shell written in C for learning Linux systems programming.

## Goals

The goal of this project is to build a small Unix-like shell while learning about:

- Processes
- System calls
- Command execution
- Pipes
- I/O redirection
- Signal handling
- Process management
- Basic shell parsing

## Features

- Execute external commands
- Built-in commands:
  - `cd`
  - `pwd`
  - `exit`
- Change directories with `cd`
- Return to the previous directory with `cd -`
- Use `cd` with no arguments to return to the home directory
- Multiple pipelines using `|`
- Input redirection using `<`
- Output redirection using `>`
- Redirection combined with pipelines
- Background execution using `&`
- Background process start and completion messages
- `Ctrl+C` handling for foreground commands and pipelines
- Single-quoted and double-quoted arguments
- Mixed quoted and unquoted text in the same argument
- Basic escaped characters, including escaped quotes and spaces
- Prompt displays the current working directory
- Basic syntax validation for invalid pipelines and unmatched quotes

## Project Structure

```text
mini-shell/
├── include/
│   ├── builtins.h
│   ├── executor.h
│   └── parser.h
├── src/
│   ├── builtins.c
│   ├── executor.c
│   ├── main.c
│   └── parser.c
├── tests/
├── .gitignore
├── Makefile
└── README.md