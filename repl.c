/*! \file
 * This file implements the Read-Eval-Print Loop (REPL) for the simple CS24
 * Python interpreter.
 */

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <malloc.h>

#include "eval.h"
#include "global.h"
#include "myalloc.h"
#include "parse.h"

void read_eval_print_loop() {
    char *line;
    size_t size;

    MEMORY_SIZE = 0x0fff;
    init_myalloc();

    while (true) {
        printf("> ");
        line = NULL;
        size = 0;

        if (getline(&line, &size, stdin) == -1) {
            // Probably end of file?
            // Hard to check.
            free(line);
            break;
        }

        if (setjmp(error_jmp)) {
            goto free_loop;
        }

        ParseStatement *stmt = read(line);

        if (stmt == NULL) {
            goto free_loop;
        }

        eval_stmt(stmt);

        memdump();

free_loop:
        free(line);
        parse_free_all();
    }
}

int main() {
    read_eval_print_loop();
    return 0;
}
