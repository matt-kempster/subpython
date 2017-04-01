#include <stdio.h>
#include <ctype.h>
#include <malloc.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "alloc.h"
#include "parse.h"
#include "types.h"

/*!
 * An enumeration of the various types of tokens the parser can generate.
 */
typedef enum TokenType {
    STREAM_END,  /*!< Hit end of line or EOF while trying to parse. */

    /* TODO: when more keywords added (lambda, ...), replace with KEYWORD */
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

    INTEGER,     /*!< An integer of some sort. Composed of digits 0-9. */
    STRING,      /*!< A string-literal. Composed of any characters. */
    IDENT,       /*!< An identifier for a variable. */

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

int is_ident_char(int ch) {
    return (isalnum(ch) || ch == '_');
}

/*!
 *
 */
TokenType next_token() {
    char ch, quote_char;
    bool keep_going;
    int i;
    int (*pred)(int);

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
        /* TODO: replace this with a lookup table */
        keep_going = false;
        switch (ch) {
            case ')':
                curr_token.type = RPAREN;
                break;
            case '(':
                curr_token.type = LPAREN;
                break;
            case ']':
                curr_token.type = RBRACKET;
                break;
            case '[':
                curr_token.type = LBRACKET;
                break;
            case '}':
                curr_token.type = RBRACE;
                break;
            case '{':
                curr_token.type = LBRACE;
                break;
            case ':':
                curr_token.type = COLON;
                break;
            case '+':
                curr_token.type = PLUS;
                break;
            case '-':
                curr_token.type = MINUS;
                break;
            case '*':
                curr_token.type = ASTERISK;
                break;
            case '/':
                curr_token.type = SLASH;
                break;
            case '.':
                curr_token.type = DOT;
                break;
            case ',':
                curr_token.type = COMMA;
                break;
            default:
                keep_going = true;
                break;
        }

        if (keep_going) {
            /**
             * Need to read a string of some sort; an integer literal, a string
             * literal (contained within quotes), or an identifier for a
             * variable.
             */
            if (ch == '\"' || ch == '\'') {
                /* Quoted string, aka string literal. */
                curr_token.type = STRING;
                quote_char = ch;

                for (i = 0; i < MAX_LENGTH; i++) {
                    ch = fgetc(stdin);

                    if (ch == EOF || ch == '\r' || ch == '\n') {
                        fprintf(stderr, "Keep strings to one line for now!\n");
                        curr_token.type = ERROR;
                    }

                    if (ch == quote_char) {
                        curr_token.string[i] = '\0';
                        break;
                    }

                    curr_token.string[i] = ch;
                }
            } else {
                /**
                 * Need to read an integer or an ident. Integers are composed
                 * entirely of digits 0-9, while idents cannot start with
                 * digits and must be composed entirely of alphanumerics and
                 * underscores.
                 */
                if (isdigit(ch)) {
                    curr_token.type = INTEGER;
                    pred = &isdigit;
                } else if (isalpha(ch) || ch == '_') {
                    curr_token.type = IDENT;
                    pred = &is_ident_char;
                } else {
                    curr_token.type = ERROR;
                    goto Done;
                }

                curr_token.string[0] = ch;
                for (i = 1; i < MAX_LENGTH; i++) {
                    ch = fgetc(stdin);

                    if (ch == EOF || ch == '\r' || ch == '\n' || isspace(ch)) {
                        curr_token.string[i] = '\0';
                        break;
                    }

                    if (!pred(ch)) {
                        curr_token.type = ERROR;
                        break;
                    }

                    curr_token.string[i] = ch;
                }
            }
        }
    }

Done:
    return curr_token.type;
}

/* New section! */

/*!
 * A constant for how many times a function can recursively call itself or
 * any function that (ultimately) called it.
 */
#define MAX_RECURSION 16

Expression *read_expression();
Primary *read_primary();
Target *read_target();

bool accept(TokenType t) {
    if (curr_token.type == t) {
        next_token();
        return true;
    }
    return false;
}

bool expect(TokenType t) {
    return accept(t);
}

Literal *read_literal() {
    int value;
    char *string = NULL;

    if (accept(INTEGER)) {
        value = atoi(curr_token.string);
        /* Sadly, we here cast an integer to a (void *)... */
        return make_literal(T_Int, (void *) value);
    }

    if (accept(STRING)) {
        string = strdup(curr_token.string);
        return make_literal(T_String, (void *) string);
    }

    return NULL;
}

Dictionary *read_dictionary(int *length) {
    Expression **keys = NULL;
    Expression **values = NULL;
    Expression *curr_key;
    Expression *curr_value;
    int max_length = 128;
    int i = 0;
    int j;

    if (!expect(LBRACE)) {
        return NULL;
    }

    keys = (Expression **) malloc(max_length * sizeof(Expression *));
    values = (Expression **) malloc(max_length * sizeof(Expression *));

    while (!accept(RBRACE)) {
        curr_key = read_expression();

        if (curr_key == NULL) {
            goto free_all;
        }

        if (!expect(COLON)) {
            free(curr_key);
            goto free_all;
        }

        curr_value = read_expression();

        if (curr_value == NULL) {
            free(curr_key);
            goto free_all;
        } else {
            keys[i] = curr_key;
            values[i] = curr_value;
            i += 1;
        }
    }

    *length = i;
    return make_dictionary(keys, values, *length);

free_all:
    for (j = 0; j < i; j++) {
        free(keys[j]);
        free(values[j]);
    }
    free(keys);
    free(values);

    return NULL;
}

UnaryExpression *read_unary_expr() {
    bool is_negated = false;
    Primary *primary;

    if (accept(MINUS)) {
        is_negated = true;
    }

    if (accept(PLUS)) {
        ;
    }

    primary = read_primary();
    if (primary == NULL) {
        return NULL;
    } else {
        return make_unary_expr(primary, is_negated);
    }
}

MultiplicativeExpression *read_multiplicative_expr(int calls) {
    MultiplicativeExpression *left;
    MultiplicativeOperation op;
    UnaryExpression *right;

    if (calls == 0) {
        return NULL;
    }

    right = read_unary_expr();
    if (right == NULL) {
        left = read_multiplicative_expr(calls - 1);
        if (left == NULL) {
            /* Too many recursive calls or malformed expression. */
            return NULL;
        } else {
            if (accept(ASTERISK)) {
                op = T_Multiply;
            } else if (accept(SLASH)) {
                op = T_Divide;
            } else {
                free(left);
                return NULL;
            }

            right = read_unary_expr();
            if (right == NULL) {
                return NULL;
            } else {
                return make_multiplicative_expr(left, op, right);
            }
        }
    } else {
        op = T_Identity;
        return make_multiplicative_expr(NULL, op, right);
    }
}

AdditiveSubexpression *read_additive_expr(int calls) {
    AdditiveSubexpression *left;
    AdditiveOperation op;
    MultiplicativeExpression *right;

    if (calls == 0) {
        return NULL;
    }

    right = read_multiplicative_expr(MAX_RECURSION);
    if (right == NULL) {
        left = read_additive_expr(calls - 1);
        if (left == NULL) {
            /* Too many recursive calls or malformed expression. */
            return NULL;
        } else {
            if (accept(PLUS)) {
                op = T_Add;
            } else if (accept(MINUS)) {
                op = T_Subtract;
            } else {
                free(left);
                return NULL;
            }

            right = read_multiplicative_expr(MAX_RECURSION);
            if (right == NULL) {
                return NULL;
            } else {
                return make_additive_expr(left, op, right);
            }
        }
    } else {
        op = T_Zero;
        return make_additive_expr(NULL, op, right);
    }
}

Expression *read_expression() {
    AdditiveSubexpression *additive_expr = read_additive_expr(MAX_RECURSION);

    if (additive_expr == NULL) {
        return NULL;
    }
    return make_expression(additive_expr);
}

/**
 * TODO: (Very high priority!) I just copy/pasted this from read_target_list
 * and that's a bad thing. Fix that.
 */
Expression **read_expression_list(int *length) {
    int max_list_length = 128;
    Expression *expression;
    Expression **expression_list =
        (Expression **) malloc(max_list_length * sizeof(Expression *));
    int i = 0;
    bool trailing_comma = false;
    int j;

    /* Any good expression list has to have at least one expression! */
    expression = read_expression();
    if (expression == NULL) {
        /* This was an invalid expression. Don't leak memory. */
        goto free_list;
    } else {
        /* Okay, so far so good - we have a first expression. */
        expression_list[i] = expression;
        i += 1;
    }

    /* Every expression from now on has a preceding comma. */
    while (accept(COMMA)) {
        /* Try to read a expression... */
        expression = read_expression();
        if (expression == NULL) {
            /**
             * No expression was read, but a comma was. This means that the
             * previous expression should be the last one, and the comma is a
             * trailing comma. However, if the last comma was supposed to
             * be a trailing comma, that's not good, so we should exit.
             */
            if (trailing_comma) {
                /* Can't have two trailing commas in a row. */
                fprintf(stderr, "SyntaxError: while reading expression list");
                goto free_expressions;
            } else {
                /* Hopefully this is the end of input. */
                trailing_comma = true;
            }
        } else {
            /* We read a expression - put it into the list and move on. */
            expression_list[i] = expression;
            i += 1;

            /**
             * TODO: If i is too big, we should use realloc to make more
             * space in the list, but right now I don't care.
             */
            if (i >= max_list_length) {
                fprintf(stderr, "LengthError: too many expressions "
                    "in expression list");
                goto free_expressions;
            }
        }
    }

    /* Everything worked! Don't forget to give back the length. */
    *length = i;
    return expression_list;

free_expressions:
    /* Need to free every expression we came up with. */
    for (j = 0; j < i; j++) {
        free(expression_list[j]);
    }

free_list:
    /* Now, get rid of the list and call it a failed day. */
    free(expression_list);
    return NULL;

}

Enclosure *read_enclosure() {
    Dictionary *dictionary;
    Expression **expression_list;
    int length;

    dictionary = read_dictionary(&length);
    if (dictionary != NULL) {
        return make_enclosure(T_Dict, dictionary, length);
    }

    if (accept(LPAREN)) {
        expression_list = read_expression_list(&length);
        if (expression_list != NULL && accept(RPAREN)) {
            return make_enclosure(T_ParentheticalForm, expression_list, length);
        } else if (expression_list != NULL) {
            /* free */
        }
    } else if (accept(LBRACKET)) {
        expression_list = read_expression_list(&length);
        if (expression_list != NULL && accept(RBRACKET)) {
            return make_enclosure(T_List, expression_list, length);
        } else if (expression_list != NULL) {
            /* free */
        }
    }

    return NULL;
}

Atom *read_atom() {
    Literal *literal;
    Enclosure *enclosure;

    literal = read_literal();
    if (literal != NULL) {
        return make_atom(T_Literal, literal);
    }

    enclosure = read_enclosure();
    if (enclosure != NULL) {
        return make_atom(T_Enclosure, enclosure);
    }

    return NULL;
}

Primary *read_primary() {
    Atom *atom;
    Target *target;

    atom = read_atom();
    if (atom != NULL) {
        return make_primary(true, atom);
    }

    target = read_target();
    if (target != NULL) {
        return make_primary(false, target);
    }

    return NULL;
}

char *read_identifier() {
    if (accept(IDENT)) {
        return strdup(curr_token.string);
    }
    return NULL;
}

AttributeReference *read_attributeref() {
    Primary *primary;
    char *identifier;

    primary = read_primary();
    if (primary == NULL) {
        return NULL;
    }
    if (!expect(DOT)) {
        return NULL;
    }
    identifier = read_identifier();
    if (identifier == NULL) {
        free(primary);
        return NULL;
    }

    return make_attributeref(primary, identifier);
}

Subscription *read_subscription() {
    Primary *primary;
    Expression **expression_list;
    int length;

    primary = read_primary();
    if (primary == NULL) {
        return NULL;
    }

    expression_list = read_expression_list(&length);
    if (expression_list == NULL) {
        return NULL;
    }

    return make_subscription(primary, expression_list, length);
}

Target *read_target() {
    AttributeReference *attributeref;
    Subscription *subscription;
    char *identifier;

    attributeref = read_attributeref();
    if (attributeref != NULL) {
        return make_target(T_AttributeReference, attributeref);
    }

    subscription = read_subscription();
    if (subscription != NULL) {
        return make_target(T_Subscription, subscription);
    }

    identifier = read_identifier();
    if (identifier != NULL) {
        return make_target(T_Identifier, identifier);
    } else {
        return NULL;
    }
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
    Statement *stmt = NULL;
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

    /* TODO: every other type of statement... */

    return stmt;
}
