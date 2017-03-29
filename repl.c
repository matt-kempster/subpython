/*! \file
 * This file implements the Read-Eval-Print Loop (REPL) for the simple CS24
 * Python interpreter.
 */

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>

#include "parse.h"

void read_eval_print_loop() {
    Statement *stmt;

    while (true) {
        fprintf(stdout, "> ");

        stmt = read_statement();
    }


}

int main() {
    read_eval_print_loop();

    return 0;
}
