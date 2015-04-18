#include "readLine.h"

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include <readline/readline.h>
#include <readline/history.h>
#include <readline/tilde.h>

#define HISTORY_FILE "~/.bas-history"

const char* historyPath = NULL;

const char* readLine(const char* prompt)
{
    if (historyPath == NULL) {
        historyPath = tilde_expand(HISTORY_FILE);
        if (read_history(historyPath) != 0) {
            // create the history file if it doesn't seem to exist
            fclose(fopen(historyPath, "a"));
        }
    }

    const char *line = readline(prompt);
    if (line != NULL) {
        add_history(line); // Add input to in-memory history
        append_history(1, historyPath);
    }
    return line;
}
