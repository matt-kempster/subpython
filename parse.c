#include <stdio.h>
#include <ctype.h>
#include <malloc.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "global.h"
#include "parse.h"

///////////////////// LEXING /////////////////////

//TODO: do we want a max line size and a statically allocated buffer, or a
// manually allocated buffer, and an infinite line size? hmmm...
static char *current_string = NULL;
static int idx;
static char current, next;

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

///////////////////// TOKENIZING /////////////////////

/*! The current token while parsing. */
static Token curr_token;

void read_string();
void read_float();
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
                    read_float();
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
        bump_char();
    }
    bump_char();

    curr_token.string[string_idx] = '\0';
    curr_token.type = STRING;
}

void read_float() {
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
        curr_token.float_value = atoi(curr_token.string);
        curr_token.type = FLOAT;
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

ParseStatement *read_statement();

ParseExpression *read_expression();
ParseExpression *read_literal();
ParseExpression *read_paren_expression();
ParseExpression *read_list_literal();
ParseExpression *read_dict_literal();
bool is_lval(ParseExpression *);
bool is_stmt(ParseExpression *);

int get_precedence(TokenType);
bool is_operator(TokenType);
bool is_right_assoc(TokenType);
ExpressionType expression_type(TokenType);

/*! Serves as the entrypoint into the parser, taking ownership of
    the string. */
ParseStatement *read(char *string) {
    init_lex(string);
    bump_token();
    return read_statement();
}

ParseStatement *read_statement() {
    ParseStatement *stmt;

    if (try_consume(LINE_END)) {
        return NULL;
    } else if (try_consume(DEL)) {
        expect(IDENT);
        stmt = parse_alloc(sizeof(ParseStatement));
        stmt->type = STMT_DEL;
        stmt->identifier = curr_token.string;
        bump_token();
        expect_consume(LINE_END);
    } else {
        // We need to parse an ParseExpression ParseStatement.
        ParseExpression *expr = read_expression(PRECEDENCE_LOWEST);

        stmt = parse_alloc(sizeof(ParseStatement));
        stmt->type = STMT_EXPR;
        stmt->expr = expr;
        expect_consume(LINE_END);
    }

    return stmt;
}

ParseExpression *read_expression(int precedence) {
    ParseExpression *lhs = read_literal();

    while (is_operator(curr_token.type)) {
        if (try_consume(LBRACKET)) {
            ParseExpression *subscript = read_expression(PRECEDENCE_LOWEST);
            ParseExpression *expr = parse_alloc(sizeof(ParseExpression));
            expr->type = EXPR_SUBSCRIPT;
            expr->lhs = lhs;
            expr->rhs = subscript;
            expect_consume(RBRACKET);
            lhs = expr;
        } else {
            int new_precedence = get_precedence(curr_token.type);

            if (new_precedence < precedence) {
                break;
            }

            TokenType op_type = curr_token.type;

            if (op_type == EQUAL && !is_lval(lhs)) {
                error(curr_token.pos, "LHS is not an L-Value (assignable).");
            }

            bump_token();

            ParseExpression *rhs = read_expression(new_precedence +
                                              is_right_assoc(op_type) ? 0 : 1);
            ParseExpression *expr = parse_alloc(sizeof(ParseExpression));
            expr->type = expression_type(op_type);
            expr->lhs = lhs;
            expr->rhs = rhs;
            lhs = expr;
        }
    }

    return lhs;
}

ParseExpression *read_literal() {
    switch (curr_token.type) {
        case MINUS:
            bump_token();
            ParseExpression *expr = parse_alloc(sizeof(ParseExpression));
            expr->type = EXPR_NEGATE;
            expr->lhs = read_expression(PRECEDENCE_UNARY_NEG);
            expr->rhs = NULL;
            return expr;

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
            expr = parse_alloc(sizeof(ParseExpression));
            expr->type = EXPR_IDENT;
            expr->string = parse_string_dup(curr_token.string);
            bump_token();
            return expr;

        case FLOAT:
            expr = parse_alloc(sizeof(ParseExpression));
            expr->type = EXPR_FLOAT;
            expr->float_value = curr_token.float_value;
            bump_token();
            return expr;

        case STRING:
            expr = parse_alloc(sizeof(ParseExpression));
            expr->type = EXPR_STRING;
            expr->string = parse_string_dup(curr_token.string);
            bump_token();
            return expr;

        default:
            error(curr_token.pos, "Unexpected token while reading ParseExpression "
                                  "literal.");
            return NULL;
    }
}

ParseExpression *read_paren_expression() {
    expect_consume(LPAREN);
    ParseExpression *expr = read_expression(PRECEDENCE_LOWEST);
    expect_consume(RPAREN);
    return expr;
}

ParseExpression *read_list_literal() {
    // We implicitly reverse the list in this method, but it'll be reversed
    // when we initialize our list in eval.c!

    bool first = true;
    ParseListNode *list = NULL;
    expect_consume(LBRACKET);

    while (!try_consume(RBRACKET)) {
        if (first) {
            first = false;
        } else {
            expect_consume(COMMA);
        }

        ParseListNode *next = parse_alloc(sizeof(ParseListNode));
        next->next = list;
        next->expr = read_expression(PRECEDENCE_LOWEST);
        list = next;
    }

    ParseExpression *expr = parse_alloc(sizeof(ParseExpression));
    expr->type = EXPR_LIST;
    expr->list = list;
    return expr;
}

ParseExpression *read_dict_literal() {
    bool first = true;
    ParseDictNode *dict = NULL;
    expect_consume(LBRACE);

    while (!try_consume(RBRACE)) {
        if (first) {
            first = false;
        } else {
            expect_consume(COMMA);
        }

        ParseDictNode *next = parse_alloc(sizeof(ParseDictNode));
        next->next = dict;
        next->key = read_expression(PRECEDENCE_LOWEST);
        expect_consume(COLON);
        next->value = read_expression(PRECEDENCE_LOWEST);
        dict = next;
    }

    ParseExpression *expr = parse_alloc(sizeof(ParseExpression));
    expr->type = EXPR_DICT;
    expr->dict = dict;
    return expr;
}

bool is_lval(ParseExpression *expr) {
    return expr->type == EXPR_SUBSCRIPT || expr->type == EXPR_IDENT;
}

bool is_stmt(ParseExpression *expr) {
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
