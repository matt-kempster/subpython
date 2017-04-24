/*! \file
 * This file contains the important global definitions for the CS24 Python
 * interpreter.
 */

#ifndef GLOBAL_H
#define GLOBAL_H

#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <stdbool.h>
#include <setjmp.h>

/*! Maximum length of a single token. */
#define MAX_LENGTH 512

/*! Default initial size for realloc-growing arrays. */
#define INITIAL_SIZE 8

/* A handy macro to delineate an unreachable branch in switches. */
#define UNREACHABLE() \
  { fprintf(stderr, "THIS SHOULD BE UNREACHABLE!"); exit(-1); }

void *parse_alloc(size_t sz);
void parse_free_all();
char *parse_string_dup(const char *str);

//TODO: where do I put this???
void error(int pos, const char *fmt, ...) __attribute__((noreturn));
extern sigjmp_buf error_jmp;

/********************* PARSE TYPES *********************/

/*!
 * An enumeration of the various types of tokens the parser can generate.
 */
typedef enum TokenType {
    STREAM_END,  /*!< EOF. */
    LINE_END,    /*!< Hit end of line. */

    DEL,         /*!< Deletion keyword. */

    RPAREN,      /*!< Right parenthesis. */
    LPAREN,      /*!< Left parenthesis. */

    LBRACKET,    /*!< Left bracket. */
    RBRACKET,    /*!< Right bracket. */

    LBRACE,      /*!< Left brace. (This character: { ) */
    RBRACE,      /*!< Right brace. (This character: } ) */
    COLON,       /*!< Colon. */

    EQUAL,       /*!< Equal sign. */
    PLUS,        /*!< Plus. */
    MINUS,       /*!< Minus.  */
    ASTERISK,    /*!< Asterisk. */
    SLASH,       /*!< Forward slash. (This character: / ) */

    DOT,         /*!< Period. */

    COMMA,       /*!< Comma. */

    FLOAT,       /*!< A float number of some sort. */
    STRING,      /*!< A string-literal. Composed of any characters. */
    IDENT,       /*!< An identifier for a variable. */
} TokenType;

typedef struct Token {
    TokenType type;
    int pos;
    char string[MAX_LENGTH];
    int int_value;
    float float_value;
} Token;

typedef enum StatementType {
    STMT_DEL,
    STMT_EXPR
} StatementType;

typedef struct ParseStatement {
    enum StatementType type;

    union {
        struct ParseExpression *expr;
        char *identifier;
    };
} ParseStatement;

typedef enum ExpressionType {
    EXPR_SUBSCRIPT,
    EXPR_NEGATE,
    EXPR_IDENT,
    EXPR_STRING,
    EXPR_FLOAT,
    EXPR_LIST,
    EXPR_DICT,
    EXPR_ASSIGN,
    EXPR_ADD,
    EXPR_SUB,
    EXPR_MULT,
    EXPR_DIV
} ExpressionType;

typedef struct ParseExpression {
    enum ExpressionType type;

    union {
        struct {
            struct ParseExpression *lhs, *rhs;
        };
        char *string;
        float float_value;
        struct ParseListNode *list;
        struct ParseDictNode *dict;
    };
} ParseExpression;

typedef struct ParseListNode {
    struct ParseListNode *next;
    struct ParseExpression *expr;
} ParseListNode;

typedef struct ParseDictNode {
    struct ParseDictNode *next;
    struct ParseExpression *key, *value;
} ParseDictNode;

#endif /* GLOBAL_H */
