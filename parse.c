#include <stdio.h>
#include <ctype.h>
#include <malloc.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "alloc.h"
#include "parse.h"
#include "lex.h"
#include "types.h"

/* A handy macro to delineate an unreachable branch in switches. */
#define UNREACHABLE() \
  { fprintf(stderr, "THIS SHOULD BE UNREACHABLE!"); exit(-1); }

/*! The current token while parsing. */
static Token curr_token;

void read_string();
void read_int();
void read_identifier();

/*!
 * Moves the "token pointer" one token ahead on the current character stream.
 */
void bump_token() {
    // For now, eat all spaces before the token.
    while (curr_char() == ' ' || curr_char() == '\t') {
        bump_char();
    }

    curr_token.pos = curr_pos();

    switch (curr_char()) {
        case '\0':
        case EOF:
            bump_char();
            curr_token.type = STREAM_END;

        case '\n':
            bump_char();
            curr_token.type = LINE_END;
            break;

        case '(':
            bump_char();
            curr_token.type = LPAREN;
            break;

        case ')':
            bump_char();
            curr_token.type = RPAREN;
            break;

        case '[':
            bump_char();
            curr_token.type = LBRACKET;
            break;

        case ']':
            bump_char();
            curr_token.type = RBRACKET;
            break;

        case '{':
            bump_char();
            curr_token.type = LBRACE;
            break;

        case '}':
            bump_char();
            curr_token.type = RBRACE;
            break;

        case ':':
            bump_char();
            curr_token.type = COLON;
            break;

        case '*':
            bump_char();
            curr_token.type = ASTERISK;
            break;

        case '/':
            bump_char();
            curr_token.type = SLASH;
            break;

        case '.':
            bump_char();
            curr_token.type = DOT;
            break;

        case ',':
            bump_char();
            curr_token.type = COMMA;
            break;

        case '+':
            bump_char();
            curr_token.type = PLUS;
            break;

        case '-':
            bump_char();
            curr_token.type = MINUS;
            break;

        case '=':
            bump_char();
            curr_token.type = EQUAL;
            break;

        case '\'':
        case '\"':
            read_string();
            break;

        default: {
                if (isdigit(curr_char())) {
                    read_int();
                } else if (isalpha(curr_char()) || curr_char() == '_') {
                    read_identifier();
                } else {
                    error(curr_pos(), "Unknown token");
                }
            }
            break;
    }
}

void read_string() {
    int string_idx = 0;
    char start = curr_char();
    bump_char();

    while (curr_char() != start) {
        //TODO: easy to add escapes.
        curr_token.string[string_idx++] = curr_char();
    }

    bump_char();
    curr_token.type = STRING;
}

void read_int() {
    int string_idx = 0;

    while (isdigit(curr_char())) {
        //TODO: size limit
        curr_token.string[string_idx++] = curr_char();
        bump_char();
    }

    if (curr_char() == '.' && isdigit(next_char())) {
        bump_char();
        curr_token.string[string_idx++] = '.';

        while (isdigit(curr_char())) {
            //TODO: size limit
            curr_token.string[string_idx++] = curr_char();
            bump_char();
        }

        curr_token.string[string_idx] = '\0';
        curr_token.float_value = atof(curr_token.string);
        curr_token.type = FLOAT;
    } else {
        curr_token.string[string_idx] = '\0';
        curr_token.int_value = atoi(curr_token.string);
        curr_token.type = INTEGER;
    }
}

void read_identifier() {
    int string_idx = 0;

    while (isalnum(curr_char()) || curr_char() == '_') {
        curr_token.string[string_idx++] = curr_char();
        bump_char();
    }

    curr_token.string[string_idx] = '\0';

    if (strcmp(curr_token.string, "del") == 0) {
        curr_token.type = DEL;
    } else {
        curr_token.type = IDENT;
    }
}

/*! Bumps the token stream if the current token matches type T,
    returning whether the token matched. */
bool try_consume(TokenType t) {
    if (curr_token.type == t) {
        bump_token();
        return true;
    } else {
        return false;
    }
}

/*! Expects a token, otherwise throws an error. */
void expect(TokenType t) {
    if (curr_token.type != t) {
        //TODO: make this actual strings...
        error(curr_token.pos, "Expected token %d, got %d.", t, curr_token.type);
    }
}

/*! Expects a token, bumping if it was found, otherwise throws an
    error. */
void expect_consume(TokenType t) {
    expect(t);
    bump_token();
}

///////////////////// PARSING /////////////////////

Statement *read_statement();

Expression *read_expression();
Expression *read_literal();
Expression *read_paren_expression();
Expression *read_list_literal();
Expression *read_dict_literal();
bool is_lval(Expression *);
bool is_stmt(Expression *);

int get_precedence(TokenType);
bool is_operator(TokenType);
bool is_right_assoc(TokenType);
ExpressionType expression_type(TokenType);

/*! Serves as the entrypoint into the parser, taking ownership of
    the string. */
Statement *read(char *string) {
    init_lex(string);
    bump_token();
    return read_statement();
}

Statement *read_statement() {
    Statement *stmt;

    if (try_consume(LINE_END)) {
        return NULL;
    } else if (try_consume(DEL)) {
        int expr_pos = curr_token.pos;
        Expression *expr = read_expression(PRECEDENCE_LOWEST);

        if (!is_lval(expr)) {
            error(expr_pos, "Expected lval expression for `del`.");
        }

        stmt = make_statement_del(expr);
        expect_consume(LINE_END);
    } else {
        int expr_pos = curr_token.pos;
        // We need to parse an expression statement.
        Expression *expr = read_expression(PRECEDENCE_LOWEST);

        if (!is_stmt(expr)) {
            error(expr_pos, "Expected expression-statement.");
        }

        stmt = make_statement_expr(expr);
        expect_consume(LINE_END);
    }

    return stmt;
}

Expression *read_expression(int precedence) {
    Expression *lhs = read_literal();

    while (is_operator(curr_token.type)) {
        if (try_consume(LBRACKET)) {
            Expression *subscript = read_expression(PRECEDENCE_LOWEST);
            expect_consume(RBRACKET);
            lhs = make_expression_subscription(lhs, subscript);
        } else {
            int new_precedence = get_precedence(curr_token.type);

            if (new_precedence < precedence) {
                break;
            }

            TokenType op_type = curr_token.type;
            bump_token();

            if (is_right_assoc(op_type)) {
                Expression *rhs = read_expression(new_precedence);
                lhs = make_expression_binary(expression_type(op_type), lhs, rhs);
            } else {
                Expression *rhs = read_expression(new_precedence + 1);
                lhs = make_expression_binary(expression_type(op_type), lhs, rhs);
            }
        }
    }

    return lhs;
}

Expression *read_literal() {
    switch (curr_token.type) {
        case MINUS:
            bump_token();
            Expression *expr = read_expression(PRECEDENCE_UNARY_NEG);
            return make_expression_negative(expr);

        case PLUS:
            bump_token();
            return read_expression(PRECEDENCE_UNARY_NEG);

        case LPAREN:
            return read_paren_expression();

        case LBRACKET:
            return read_list_literal();

        case LBRACE:
            return read_dict_literal();

        case IDENT:
            expr = make_expression_ident(curr_token.string);
            bump_token();
            return expr;

        case INTEGER:
            expr = make_expression_integer(curr_token.int_value);
            bump_token();
            return expr;

        case FLOAT:
            expr = make_expression_float(curr_token.float_value);
            bump_token();
            return expr;

        case STRING:
            expr = make_expression_string(curr_token.string);
            bump_token();
            return expr;

        default:
            error(curr_token.pos, "Unexpected token while reading expression literal.");
            return NULL;
    }
}

Expression *read_paren_expression() {
    expect_consume(LPAREN);
    Expression *expr = read_expression(PRECEDENCE_LOWEST);
    expect_consume(RPAREN);
    //TODO: this can be easily extended to tuples.
    return expr;
}

Expression *read_list_literal() {
    bool first = true;
    //TODO: explain why it's TOTALLY OKAY to reverse the list here!
    ListNode *list = NULL;
    expect_consume(LBRACKET);

    while (!try_consume(RBRACKET)) {
        if (first) {
            first = false;
        } else {
            expect_consume(COMMA);
        }

        Expression *expr = read_expression(PRECEDENCE_LOWEST);
        list = make_list(list, expr);
    }

    return make_expression_list(list);
}

Expression *read_dict_literal() {
    bool first = true;
    //TODO: explain why it's TOTALLY OKAY to reverse the dict here!
    DictNode *dict = NULL;
    expect_consume(LBRACE);

    while (!try_consume(RBRACE)) {
        if (first) {
            first = false;
        } else {
            expect_consume(COMMA);
        }

        Expression *key = read_expression(PRECEDENCE_LOWEST);
        expect_consume(COLON);
        Expression *value = read_expression(PRECEDENCE_LOWEST);
        dict = make_dict(dict, key, value);
    }

    return make_expression_dict(dict);
}

bool is_lval(Expression *expr) {
    return expr->type == EXPR_SUBSCRIPT || expr->type == EXPR_IDENT;
}
bool is_stmt(Expression *expr) {
    return expr->type == EXPR_ASSIGN;
}

int get_precedence(TokenType t) {
    switch (t) {
        case ASTERISK:
        case SLASH:
            return PRECEDENCE_MULT;

        case PLUS:
        case MINUS:
            return PRECEDENCE_PLUS;

        case EQUAL:
            return PRECEDENCE_ASSIGN;

        default:
            UNREACHABLE();
    }
}

bool is_operator(TokenType t) {
    switch (t) {
        case LPAREN:
        case LBRACKET:
        case PLUS:
        case MINUS:
        case ASTERISK:
        case EQUAL:
        case SLASH:
            return true;

        default:
            return false;
    }
}

ExpressionType expression_type(TokenType t) {
    switch (t) {
        case LBRACKET:
            return EXPR_SUBSCRIPT;

        case PLUS:
            return EXPR_ADD;

        case MINUS:
            return EXPR_SUB;

        case ASTERISK:
            return EXPR_MULT;

        case EQUAL:
            return EXPR_ASSIGN;

        case SLASH:
            return EXPR_DIV;

        default:
            UNREACHABLE();
    }
}

bool is_right_assoc(TokenType t) {
    return t == EQUAL;
}
