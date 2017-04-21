#include <stdio.h>

#include "print.h"

void print_statement(Statement *stmt) {
    if (stmt == NULL) {
        printf("NO STATEMENT\n");
        return;
    }

    switch (stmt->type) {
        case STMT_DEL:
            printf("del ");
            print_expression(stmt->expr);
            break;

        case STMT_EXPR:
            print_expression(stmt->expr);
            break;
    }

    printf("\n");
}

void print_expression(Expression *expr) {
    switch (expr->type) {
        case EXPR_SUBSCRIPT:
            printf("(");
            print_expression(expr->lhs);
            printf(")[");
            print_expression(expr->rhs);
            printf("]");
            break;

        case EXPR_NEGATE:
            printf("-(");
            print_expression(expr->lhs);
            printf(")");
            break;

        case EXPR_IDENT:
            printf("%s", expr->string);
            break;

        case EXPR_STRING:
            printf("'%s'", expr->string);
            break;

        case EXPR_INT:
            printf("%d", expr->int_value);
            break;

        case EXPR_FLOAT:
            printf("%f", expr->float_value);
            break;

        case EXPR_LIST: {
                printf("[");

                for (ListNode *i = expr->list; i != NULL; i = i->next) {
                    print_expression(i->expr);

                    if (i->next != NULL) {
                        printf(", ");
                    }
                }
            }
            break;

        case EXPR_DICT: {
                printf("[");

                for (DictNode *i = expr->dict; i != NULL; i = i->next) {
                    print_expression(i->key);
                    printf(": ");
                    print_expression(i->value);

                    if (i->next != NULL) {
                        printf(", ");
                    }
                }
            }
            break;

        case EXPR_ASSIGN:
            printf("(");
            print_expression(expr->lhs);
            printf(") = (");
            print_expression(expr->rhs);
            printf(")");
            break;

        case EXPR_ADD:
            printf("(");
            print_expression(expr->lhs);
            printf(") + (");
            print_expression(expr->rhs);
            printf(")");
            break;

        case EXPR_SUB:
            printf("(");
            print_expression(expr->lhs);
            printf(") - (");
            print_expression(expr->rhs);
            printf(")");
            break;

        case EXPR_MULT:
            printf("(");
            print_expression(expr->lhs);
            printf(") * (");
            print_expression(expr->rhs);
            printf(")");
            break;

        case EXPR_DIV:
            printf("(");
            print_expression(expr->lhs);
            printf(") / (");
            print_expression(expr->rhs);
            printf(")");
            break;
    }
}
