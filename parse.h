#ifndef PARSE_H
#define PARSE_H

#include "types.h"

enum Precedence {
    PRECEDENCE_UNARY_NEG = 4,
    PRECEDENCE_MULT = 3,
    PRECEDENCE_PLUS = 2,
    PRECEDENCE_ASSIGN = 1,
    PRECEDENCE_LOWEST = 0
};

Statement *read(char *string);

#endif /* PARSE_H */
