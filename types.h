/*! \file
 * This file contains the important type-definitions for the CS24 Python
 * interpreter.
 */

#ifndef TYPES_H
#define TYPES_H

#include <stdbool.h>

/*! Maximum length of a single token. */
#define MAX_LENGTH 512

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

    INTEGER,     /*!< An integer of some sort. Composed of digits 0-9. */
    FLOAT,       /*!< A float number of some sort. */
    STRING,      /*!< A string-literal. Composed of any characters. */
    IDENT,       /*!< An identifier for a variable. */
} TokenType;

typedef struct Token {
    TokenType type;
    char string[MAX_LENGTH];
    int int_value;
    float float_value;
} Token;

typedef enum StatementType {
    STMT_DEL,
    STMT_EXPR
} StatementType;

typedef struct Statement {
    enum StatementType type;
    struct Expression *expr;
} Statement;

typedef enum ExpressionType {
    EXPR_SUBSCRIPT,
    EXPR_NEGATE,
    EXPR_IDENT,
    EXPR_STRING,
    EXPR_INT,
    EXPR_FLOAT,
    EXPR_LIST,
    EXPR_DICT,
    EXPR_ASSIGN,
    EXPR_ADD,
    EXPR_SUB,
    EXPR_MULT,
    EXPR_DIV
} ExpressionType;

typedef struct Expression {
    enum ExpressionType type;

    union {
        struct {
            struct Expression *lhs, *rhs;
        };
        char *string;
        int int_value;
        float float_value;
        struct ListNode *list;
        struct DictNode *dict;
    };
} Expression;

typedef struct ListNode {
    struct ListNode *next;
    struct Expression *expr;
} ListNode;

typedef struct DictNode {
    struct DictNode *next;
    struct Expression *key, *value;
} DictNode;

#endif /* TYPES_H */
