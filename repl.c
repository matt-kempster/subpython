/*! \file
 * This file implements the Read-Eval-Print Loop (REPL) for the simple CS24
 * Python interpreter.
 */

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <malloc.h>

#include "lex.h"
#include "parse.h"
#include "types.h"

void read_eval_print_loop() {
    char *line;
    size_t size;

    while (true) {
        printf("> ");
        line = NULL;
        size = 0;

        if (getline(&line, &size, stdin) == -1) {
            // Probably end of file?
            // Hard to check.

            break;
        }

        printf("Begin parse.\n");
        Statement *stmt = read(line);
        free(line);
        printf("Read statement.\n");

        print_statement(stmt);
        //free_statement(stmt);
    }
}

int main() {
    read_eval_print_loop();

    return 0;
}
