#ifndef ALLOC_H
#define ALLOC_H

#include <setjmp.h>

#include "types.h"

void free_all();
char *string_dup(const char *str);

//TODO: where do I put this???
void error(int pos, const char *fmt, ...);
extern sigjmp_buf error_jmp;

Statement *make_statement_del(Expression *);
Statement *make_statement_expr(Expression *);

Expression *make_expression_subscription(Expression *, Expression *);
Expression *make_expression_binary(ExpressionType, Expression *, Expression *);
Expression *make_expression_negative(Expression *);
Expression *make_expression_ident(char *);
Expression *make_expression_string(char *);
Expression *make_expression_integer(int);
Expression *make_expression_float(float);
Expression *make_expression_list(ListNode *);
Expression *make_expression_dict(DictNode *);

ListNode *make_list(ListNode *, Expression *);
DictNode *make_dict(DictNode *, Expression *, Expression *);

#endif /* ALLOC_H */
