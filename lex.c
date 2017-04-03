#include <stdio.h>
#include <malloc.h>

#include "lex.h"

//TODO: do we want a max line size and a statically allocated buffer, or a
// manually allocated buffer, and an infinite line size? hmmm...
static char *current_string = NULL;
static int idx;
static char current, next;

void init_lex(char *new_string) {
    current_string = new_string;
    idx = 0;
    // We need to make sure that the char stream is now buffered.
    // Give the current current, next some dummy values.
    current = next = '_';
    // Then bump twice so the first char of new_string sits on current.
    bump_char();
    bump_char();
}

const char *curr_string() {
    return current_string;
}

int curr_pos() {
    return idx - 2;
}

char curr_char() {
    return current;
}

char next_char() {
    return next;
}

void bump_char() {
    if (next == '\0') {
        current = '\0';
    } else {
        current = next;
        next = current_string[idx++];
    }
}
