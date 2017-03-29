#include <stdio.h>
#include <ctype.h>
#include <malloc.h>
#include <stdbool.h>

#include "parse.h"
#include "types.h"

/*!
 * An enumeration of the various types of tokens the parser can generate.
 */
typedef enum TokenType {
    STREAM_END,  /*!< Hit end of line or EOF while trying to parse. */
    DEL,         /*!< Deletion keyword. */
    OTHER,
} TokenType;

/*! Maximum length of a single token. */
#define MAX_LENGTH 512

typedef struct Token {
    TokenType type;
    char string[MAX_LENGTH];
} Token;

/*! The current token while parsing is occurring. */
static Token curr_token;

bool is_del(char initial) {
    char ch;
    int i;
    const char *del = "el";
    if (initial == 'd') {
        for (i = 0; i < 2; i++) {
            ch = fgetc(stdin);
            if (ch != del[i]) {
                return false;
            }
        }
    } else {
        return false;
    }
    return true;
}

/*!
 *
 */
TokenType next_token() {
    char ch;

    /* Consume whitespace and comments. */
    while (1) {
        /* Consume whitespace. */
        do {
            ch = fgetc(stdin);
        } while (ch != EOF && ch != '\r' && ch != '\n' && isspace(ch));

        /* Handle case where we hit end of line or EOF while reading. */
        if (ch == EOF || ch == '\r' || ch == '\n') {
            curr_token.type = STREAM_END;
            goto Done;
        }

        /* Consume comments. */
        if (ch == '#') {
            do {
                ch = fgetc(stdin);
            } while (ch != EOF && ch != '\r' && ch != '\n' && isspace(ch));

            /* Handle case where we hit end of line or EOF while reading. */
            if (ch == EOF || ch == '\r' || ch == '\n') {
                curr_token.type = STREAM_END;
                goto Done;
            }
        } else {
            /* Not a comment - actual work to be done. */
            break;
        }
    }

    if (is_del(ch)) {
        curr_token.type = DEL;
    } else {
        curr_token.type = OTHER;
    }

Done:
    return curr_token.type;
}

/*!
 *
 */
Statement *read_statement() {
    bool done = false;
    Statement *stmt = (Statement *) malloc(sizeof(Statement));

    while (!done) {
        next_token();

        switch (curr_token.type) {

        case STREAM_END:
            /* Just return the statement. */
            done = true;
            break;

        case DEL:
            /* Deletion operator... these would obv. be in make_del() */
            stmt->type = T_DelStatement;
            stmt->del_stmt = (DelStatement *) malloc(sizeof(DelStatement));
            stmt->del_stmt->target_list = NULL;
            stmt->del_stmt->length = 0;
            break;

        case OTHER:
            /* TODO */
        default:
            /* TODO */
            break;
        }
    }

    return stmt;
}
