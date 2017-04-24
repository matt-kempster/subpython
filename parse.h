#ifndef PARSE_H
#define PARSE_H

#include "global.h"

enum Precedence {
    PRECEDENCE_UNARY_NEG = 4,
    PRECEDENCE_MULT = 3,
    PRECEDENCE_PLUS = 2,
    PRECEDENCE_ASSIGN = 1,
    PRECEDENCE_LOWEST = 0
};

ParseStatement *read(char *string);

// For `error()`.
const char *curr_string();

#endif /* PARSE_H */
