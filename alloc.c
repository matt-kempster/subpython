#include <stdarg.h>
#include <string.h>
#include <malloc.h>

#include "alloc.h"
#include "lex.h"

#define INITIAL_SIZE 8
void **ptr_array = NULL;
int allocated_objs = 0, max_objs = 0;
sigjmp_buf error_jmp;

void *alloc(size_t sz) {
    if (ptr_array == NULL) {
        ptr_array = malloc(sizeof(void *) * INITIAL_SIZE);
        max_objs = INITIAL_SIZE;
    } else if (allocated_objs == max_objs) {
        max_objs *= 2;
        ptr_array = realloc(ptr_array, sizeof(void *) * max_objs);
    }

    void *mem = malloc(sz);

    if (mem == NULL || ptr_array == NULL) {
        error(-1, "%s", "Allocation failed!");
    }

    ptr_array[allocated_objs++] = mem;
    return mem;
}

void error(int pos, const char *fmt, ...) {
    printf("%s", curr_string());

    if (pos != -1) {
        for (int i = 0; i < pos; i++)
            printf("-");

        printf("^\n");

        printf("Parse error: ");
    } else {
        printf("Error: ");
    }

    va_list argptr;
    va_start(argptr, fmt);
    vfprintf(stdout, fmt, argptr);
    va_end(argptr);

    printf("\n");

    longjmp(error_jmp, 1);
}

void free_all() {
    for (int i = 0; i < allocated_objs; i++)
        free(ptr_array[i]);

    free(ptr_array);
    ptr_array = NULL;
    allocated_objs = max_objs = 0;
}

Statement *make_statement_del(Expression *expr) {
    Statement *stmt = alloc(sizeof(Statement));
    stmt->type = STMT_DEL;
    stmt->expr = expr;
    return stmt;
}

Statement *make_statement_expr(Expression *expr) {
    Statement *stmt = alloc(sizeof(Statement));
    stmt->type = STMT_EXPR;
    stmt->expr = expr;
    return stmt;
}

Expression *make_expression_subscription(Expression *list, Expression *sub) {
    Expression *expr = alloc(sizeof(Expression));
    expr->type = EXPR_SUBSCRIPT;
    expr->lhs = list;
    expr->rhs = sub;
    return expr;
}
Expression *make_expression_binary(ExpressionType ty, Expression *lhs,
                                   Expression *rhs) {
    Expression *expr = alloc(sizeof(Expression));
    expr->type = ty;
    expr->lhs = lhs;
    expr->rhs = rhs;
    return expr;
}
Expression *make_expression_negative(Expression *subexp) {
    Expression *expr = alloc(sizeof(Expression));
    expr->type = EXPR_NEGATE;
    expr->lhs = subexp;
    expr->rhs = NULL;
    return expr;
}
Expression *make_expression_ident(char *str) {
    Expression *expr = alloc(sizeof(Expression));
    expr->type = EXPR_IDENT;
    expr->string = string_dup(str);
    return expr;
}
Expression *make_expression_string(char *str) {
    Expression *expr = alloc(sizeof(Expression));
    expr->type = EXPR_STRING;
    expr->string = string_dup(str);
    return expr;
}
Expression *make_expression_integer(int i) {
    Expression *expr = alloc(sizeof(Expression));
    expr->type = EXPR_INT;
    expr->int_value = i;
    return expr;
}
Expression *make_expression_float(float f) {
    Expression *expr = alloc(sizeof(Expression));
    expr->type = EXPR_FLOAT;
    expr->float_value = f;
    return expr;
}
Expression *make_expression_list(ListNode *l) {
    Expression *expr = alloc(sizeof(Expression));
    expr->type = EXPR_LIST;
    expr->list = l;
    return expr;
}
Expression *make_expression_dict(DictNode *d) {
    Expression *expr = alloc(sizeof(Expression));
    expr->type = EXPR_DICT;
    expr->dict = d;
    return expr;
}

ListNode *make_list(ListNode *next, Expression *expr) {
    ListNode *list = alloc(sizeof(ListNode));
    list->next = next;
    list->expr = expr;
    return list;
}

DictNode *make_dict(DictNode *next, Expression *key, Expression *value) {
    DictNode *dict = alloc(sizeof(DictNode));
    dict->next = next;
    dict->key = key;
    dict->value = value;
    return dict;
}

char *string_dup(const char *str) {
    char *str2 = alloc(sizeof(char) * (strlen(str) + 1));

    for (size_t i = 0; i < strlen(str) + 1; i++)
        str2[i] = str[i];

    return str2;
}
