#include "input.h"

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include <readline/readline.h>
#include <readline/history.h>
#include <readline/tilde.h>

#define HISTORY_FILE "~/.bas-history"

const char* historyPath = NULL;
static char* line = NULL;
static size_t lineSize = 0;

void input_init()
{
    historyPath = tilde_expand(HISTORY_FILE);
    if (read_history(historyPath) != 0) {
        // create the history file if it doesn't seem to exist
        fclose(fopen(historyPath, "a"));
    }
}

char* input_get(const char* prompt)
{
    if (isatty(STDIN_FILENO)) {
        if (line != NULL) {
            free(line);
        }
        line = readline(prompt);
        if (line != NULL) {
            add_history(line); // Add input to in-memory history
            append_history(1, historyPath);
        }
    }
    else {
        if (getline(&line, &lineSize, stdin) < 0) {
            line = NULL;
        }
    }
    return line;
}
