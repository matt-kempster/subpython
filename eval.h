#ifndef EVAL_H
#define EVAL_H

#include "global.h"

typedef int RefId;

//TODO: should these go in global?

typedef struct Reference {
    //IDEA: in the mark and sweep, they can just set occupied = false at
    // the beginning for ALL refs, then marking is just setting occupied = true.
    bool occupied;

    enum Type {
        VAL_FLOAT,
        VAL_STRING,
        VAL_LIST,
        VAL_DICT,
        VAL_EMPTY
    } type;

    union {
        float *float_value;
        int *int_value;
        char *string_value;
        struct ListNode *list;
        struct DictNode *dict;
    };
} Reference;

typedef struct ListNode {
    struct ListNode *next;
    RefId value;
} ListNode;

typedef struct DictNode {
    struct DictNode *next;
    RefId key, value;
} DictNode;

void print_ref(RefId ref, bool newline, int depth);

void eval_stmt(struct ParseStatement *stmt);
RefId eval_expr(struct ParseExpression *expr);
RefId *eval_expr_lval(struct ParseExpression *expr);

// Helpers
float eval_expect_float(struct ParseExpression *expr);
RefId *get_global_variable(char *name, bool create);
void delete_global_variable(char *name);
bool key_equals(RefId a, RefId b);
struct Reference *deref(RefId id);
struct ListNode *alloc_list_node(struct ListNode *next, RefId value);
struct DictNode *alloc_dict_node(struct DictNode *next,
                                 RefId key, RefId value);
RefId make_reference();
RefId make_reference_float(float f);
RefId make_reference_string(char *c);
RefId make_reference_list(ListNode *l);
RefId make_reference_dict(DictNode *d);
void assign_ref(RefId a, RefId b);
RefId key_clone(RefId ref);
char *eval_string_dup(char *, RefId);

#endif /* EVAL_H */
