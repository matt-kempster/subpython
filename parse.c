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

    RPAREN,      /*!< Right parenthesis. */
    LPAREN,      /*!< Left parenthesis. */

    LBRACKET,    /*!< Left bracket. */
    RBRACKET,    /*!< Right bracket. */

    LBRACE,      /*!< Left brace. (This character: { ) */
    RBRACE,      /*!< Right brace. (This character: } ) */
    COLON,       /*!< Colon. */

    PLUS,        /*!< Plus. */
    MINUS,       /*!< Minus.  */
    ASTERISK,    /*!< Asterisk. */
    SLASH,       /*!< Forward slash. (This character: / ) */

    DOT,         /*!< Period. */

    COMMA,       /*!< Comma. */

    SQUOTE,      /*!< Single-quote. */
    DQUOTE,      /*!< Double-quote. */

    INTEGER,     /*!< An integer of some sort. Contains at least 1 digit. */
    STRING,      /*!< A string of some sort. Consists of a-z, A-Z, -, _, 0-9. */

    ERROR,       /*!< Ran into a parsing error.  */
    OTHER,       /*!< Placeholder to make things compile. */
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

bool accept(TokenType t) {
    if (curr_token.type == t) {
        next_token();
        return true;
    }
    return false;
}

bool expect(TokenType t) {
    if (accept(t)) {
        return true;
    }
    return false;
}

/*!
 * Try to read a list of targets (a "target list"). A target list has this form:
 *     TARGET ("," TARGET)* [","]
 * The input parameter length must be a pointer to an integer. On successfully
 * reading a target list, this function will properly set that pointer to the
 * length of the list, and will return a pointer to that list. Otherwise, return
 * NULL.
 */
Target **read_target_list(int *length) {
    int max_list_length = 128;
    Target *target;
    Target **target_list =
        (Target **) malloc(max_list_length * sizeof(Target *));
    int i = 0;
    bool trailing_comma = false;
    int j;

    /* Any good target list has to have at least one target! */
    target = read_target();
    if (target == NULL) {
        /* This was an invalid target. Don't leak memory. */
        goto free_list;
    } else {
        /* Okay, so far so good - we have a first target. */
        target_list[i] = target;
        i += 1;
    }

    /* Every target from now on has a preceding comma. */
    while (accept(COMMA)) {
        /* Try to read a target... */
        target = read_target();
        if (target == NULL) {
            /**
             * No target was read, but a comma was. This means that the
             * previous target should be the last one, and the comma is a
             * trailing comma. However, if the last comma was supposed to
             * be a trailing comma, that's not good, so we should exit.
             */
            if (trailing_comma) {
                /* Can't have two trailing commas in a row. */
                fprintf(stderr, "SyntaxError: while reading target list");
                goto free_targets;
            } else {
                /* Hopefully this is the end of input. */
                trailing_comma = true;
            }
        } else {
            /* We read a target - put it into the list and move on. */
            target_list[i] = target;
            i += 1;

            /**
             * TODO: If i is too big, we should use realloc to make more
             * space in the list, but right now I don't care.
             */
            if (i >= max_list_length) {
                fprintf(stderr, "LengthError: too many targets in target list");
                goto free_targets;
            }
        }
    }

    /* After we're done reading commas and targets, we should be done. */
    if (!accept(STREAM_END)) {
        /* There's more to read, but there shouldn't be! */
        goto free_targets;
    }

    /* Everything worked! Don't forget to give back the length. */
    *length = i;
    return target_list;

free_targets:
    /* Need to free every target we came up with. */
    for (j = 0; j < i; j++) {
        free(target_list[j]);
    }

free_list:
    /* Now, get rid of the list and call it a failed day. */
    free(target_list);
    return NULL;

}

/*!
 *
 */
Statement *read_statement() {
    Statement *stmt;
    int length;
    Target **target_list;

    next_token();

    if (accept(DEL)) {
        /**
         * This statement will be a deletion statement; these have the form
         * "del TARGET_LIST".
         */
        target_list = read_target_list(&length);
        if (target_list == NULL) {
            return NULL;
        } else {
            stmt = (Statement *) malloc(sizeof(Statement));
            stmt->type = T_DelStatement;
            stmt->del_stmt = make_del_statement(target_list, length);
            return stmt;
        }
    }

#if 0
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
#endif

    return stmt;
}
