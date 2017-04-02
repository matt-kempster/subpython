#include <string.h>
#include <malloc.h>

#include "alloc.h"

void *malloc_assert(size_t sz) {
    void *mem = malloc(sz);

    if (mem == NULL) {
        //TODO: error!
    }

    return mem;
}

Statement *make_statement_del(Expression *expr) {
    Statement *stmt = malloc_assert(sizeof(Statement));
    stmt->type = STMT_DEL;
    stmt->expr = expr;
    return stmt;
}

Statement *make_statement_expr(Expression *expr) {
    Statement *stmt = malloc_assert(sizeof(Statement));
    stmt->type = STMT_EXPR;
    stmt->expr = expr;
    return stmt;
}

Expression *make_expression_subscription(Expression *list, Expression *sub) {
    Expression *expr = malloc_assert(sizeof(Expression));
    expr->type = EXPR_SUBSCRIPT;
    expr->lhs = list;
    expr->rhs = sub;
    return expr;
}
Expression *make_expression_binary(ExpressionType ty, Expression *lhs, Expression *rhs) {
    Expression *expr = malloc_assert(sizeof(Expression));
    expr->type = ty;
    expr->lhs = lhs;
    expr->rhs = rhs;
    return expr;
}
Expression *make_expression_negative(Expression *subexp) {
    Expression *expr = malloc_assert(sizeof(Expression));
    expr->type = EXPR_NEGATE;
    expr->lhs = subexp;
    expr->rhs = NULL;
    return expr;
}
Expression *make_expression_ident(char *str) {
    Expression *expr = malloc_assert(sizeof(Expression));
    expr->type = EXPR_IDENT;
    //TODO: strdup is not malloc safe?
    expr->string = strndup(str, MAX_LENGTH);
    return expr;
}
Expression *make_expression_string(char *str) {
    Expression *expr = malloc_assert(sizeof(Expression));
    expr->type = EXPR_STRING;
    expr->string = strndup(str, MAX_LENGTH);
    return expr;
}
Expression *make_expression_integer(int i) {
    Expression *expr = malloc_assert(sizeof(Expression));
    expr->type = EXPR_INT;
    expr->int_value = i;
    return expr;
}
Expression *make_expression_float(float f) {
    Expression *expr = malloc_assert(sizeof(Expression));
    expr->type = EXPR_FLOAT;
    expr->float_value = f;
    return expr;
}
Expression *make_expression_list(ListNode *l) {
    Expression *expr = malloc_assert(sizeof(Expression));
    expr->type = EXPR_LIST;
    expr->list = l;
    return expr;
}
Expression *make_expression_dict(DictNode *d) {
    Expression *expr = malloc_assert(sizeof(Expression));
    expr->type = EXPR_DICT;
    expr->dict = d;
    return expr;
}

ListNode *make_list(ListNode *next, Expression *expr) {
    ListNode *list = malloc_assert(sizeof(ListNode));
    list->next = next;
    list->expr = expr;
    return list;
}

DictNode *make_dict(DictNode *next, Expression *key, Expression *value) {
    DictNode *dict = malloc_assert(sizeof(DictNode));
    dict->next = next;
    dict->key = key;
    dict->value = value;
    return dict;
}
