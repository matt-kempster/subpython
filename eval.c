#include <string.h>

#include "global.h"
#include "eval.h"
#include "myalloc.h"

/* Global variable information. */

#define MAX_DEPTH 4

struct GlobalVariable {
    char *name;
    RefId ref;
} *global_vars = NULL;

int num_vars = 0;
int max_vars = 0;

struct Reference *ref_table = NULL;
int num_refs = 0;
int max_refs = 0;

//// CODE ////

void print_list(struct ListNode *initial, int depth) {
    ListNode *curr_node = initial;
    bool first = true;

    while (curr_node != NULL) {
        if (first) {
            first = false;
        } else {
            fprintf(stdout, ", ");
        }
        if (depth != 0) {
            print_ref(curr_node->value, false, depth - 1);
        } else {
            fprintf(stdout, "...");
        }
        curr_node = curr_node->next;
    }
}

void print_dict(struct DictNode *initial, int depth) {
    DictNode *curr_node = initial;
    bool first = true;

    while (curr_node != NULL) {
        if (first) {
            first = false;
        } else {
            fprintf(stdout, ", ");
        }
        print_ref(curr_node->key, false, 0); /* depth irrelevant for keys */
        fprintf(stdout, ": ");
        if (depth != 0) {
            print_ref(curr_node->value, false, depth - 1);
        } else {
            fprintf(stdout, "...");
        }
        curr_node = curr_node->next;
    }
}

void print_ref(RefId ref, bool newline, int depth) {
    switch (ref_table[ref].type) {
        case VAL_FLOAT:
            fprintf(stdout, "%f", *ref_table[ref].float_value);
            break;
        case VAL_STRING:
            fprintf(stdout, "\"%s\"", ref_table[ref].string_value);
            break;
        case VAL_LIST:
            fprintf(stdout, "[");
            print_list(ref_table[ref].list, depth);
            fprintf(stdout, "]");
            break;
        case VAL_DICT:
            fprintf(stdout, "{");
            print_dict(ref_table[ref].dict, depth);
            fprintf(stdout, "}");
            break;
        default:
            fprintf(stdout, "Unrecognized reference type\n");
            break;
    }
    if (newline) {
        fprintf(stdout, "\n");
    }
}


void eval_stmt(ParseStatement *stmt) {
    RefId eval_ref;

    switch (stmt->type) {
        case STMT_DEL:
            delete_global_variable(stmt->identifier);
            break;
        case STMT_EXPR:
            eval_ref = eval_expr(stmt->expr);
            if (stmt->expr->type != EXPR_ASSIGN) {
                print_ref(eval_ref, true, MAX_DEPTH);
            }
            break;
    }
}

RefId eval_expr(ParseExpression *expr) {
    RefId lhs, rhs;

    switch (expr->type) {
        case EXPR_SUBSCRIPT:
            lhs = eval_expr(expr->lhs);
            if (deref(lhs)->type == VAL_LIST) {
                int idx = (int) eval_expect_float(expr->rhs);
                ListNode *node = deref(lhs)->list;

                for (int i = 0; i < idx; i++) {
                    node = node->next;

                    if (node == NULL) {
                        //TODO: error
                    }
                }

                return node->value;
            } else if (deref(lhs)->type == VAL_DICT) {
                rhs = eval_expr(expr->rhs);
                DictNode *node = deref(lhs)->dict;

                while (node != NULL) {
                    if (key_equals(node->key, rhs)) {
                        break;
                    }
                    node = node->next;
                }

                if (node == NULL) {
                    error(-1, "%s", "Key cannot be found!");
                }

                return node->value;
            } else {
                //TODO: error
            }
        case EXPR_NEGATE: {
            float float_val = eval_expect_float(expr->lhs);
            return make_reference_float(-float_val);
        }
        case EXPR_IDENT:
            // We dereference, because get_global_variable returns a RefId*
            return *get_global_variable(expr->string, false);
        case EXPR_STRING:
            return make_reference_string(expr->string);
        case EXPR_FLOAT:
            return make_reference_float(expr->float_value);
        case EXPR_LIST: {
            ParseListNode *parse_list = expr->list;
            ListNode *eval_list = NULL;
            while (parse_list != NULL) {
                eval_list = alloc_list_node(eval_list,
                                            eval_expr(parse_list->expr));
                parse_list = parse_list->next;
            }
            return make_reference_list(eval_list);
        }
        case EXPR_DICT: {
            ParseDictNode *parse_dict = expr->dict;
            DictNode *eval_dict = NULL;
            while (parse_dict != NULL) {
                eval_dict = alloc_dict_node(eval_dict,
                                            key_clone(eval_expr(parse_dict->key)),
                                            eval_expr(parse_dict->value));
                parse_dict = parse_dict->next;
            }
            return make_reference_dict(eval_dict);
        }
        case EXPR_ASSIGN: {
            RefId *lval = eval_expr_lval(expr->lhs);
            rhs = eval_expr(expr->rhs);
            *lval = rhs;
            return *lval;
        }
        case EXPR_ADD: {
            //TODO: add strings, not just floats. (trivial)
            float lhs_val = eval_expect_float(expr->lhs);
            float rhs_val = eval_expect_float(expr->rhs);
            return make_reference_float(lhs_val + rhs_val);
        }
        case EXPR_SUB: {
            float lhs_val = eval_expect_float(expr->lhs);
            float rhs_val = eval_expect_float(expr->rhs);
            return make_reference_float(lhs_val - rhs_val);
        }
        case EXPR_MULT: {
            float lhs_val = eval_expect_float(expr->lhs);
            float rhs_val = eval_expect_float(expr->rhs);
            return make_reference_float(lhs_val * rhs_val);
        }
        case EXPR_DIV: {
            float lhs_val = eval_expect_float(expr->lhs);
            float rhs_val = eval_expect_float(expr->rhs);
            return make_reference_float(lhs_val / rhs_val);
        }
        default:
            UNREACHABLE();
    }
}

RefId *eval_expr_lval(ParseExpression *expr) {
    RefId lhs, rhs;

    switch (expr->type) {
        case EXPR_SUBSCRIPT:
            lhs = eval_expr(expr->lhs);
            if (deref(lhs)->type == VAL_LIST) {
                int idx = (int) eval_expect_float(expr->rhs);
                ListNode *node = deref(lhs)->list;

                for (int i = 0; i < idx; i++) {
                    node = node->next;

                    if (node == NULL) {
                        error(-1, "%s", "Index out of bounds: %d out of %d.", idx, i);
                    }
                }

                return &(node->value);
            } else if (deref(lhs)->type == VAL_DICT) {
                rhs = eval_expr(expr->rhs);
                DictNode *node = deref(lhs)->dict;

                while (node != NULL) {
                    if (key_equals(node->key, rhs)) {
                        break;
                    }

                    node = node->next;
                }

                if (node == NULL) {
                    node = alloc_dict_node(deref(lhs)->dict,
                                           key_clone(rhs),
                                           -1);
                    deref(lhs)->dict = node;
                }

                return &(node->value);
            } else {
                error(-1, "%s", "Can only subscript lists and dictionaries.");
            }
        case EXPR_IDENT:
            return get_global_variable(expr->string, true);
            break;
        case EXPR_ASSIGN: {
            RefId *lval = eval_expr_lval(expr->lhs);
            rhs = eval_expr(expr->rhs);
            *lval = rhs;
            return lval;
        }
        default:
            UNREACHABLE();
    }
}

float eval_expect_float(ParseExpression *expr) {
    RefId id = eval_expr(expr);

    if (deref(id)->type != VAL_FLOAT) {
        error(-1, "%s", "Expected numerical (float) value.");
    }

    return *deref(id)->float_value;
}

RefId *get_global_variable(char *name, bool create) {
    for (int i = 0; i < num_vars; i++) {
        if (strcmp(name, global_vars[i].name) == 0) {
            return &global_vars[i].ref;
        }
    }

    if (create) {
        if (global_vars == NULL) {
            global_vars = malloc(sizeof(struct GlobalVariable) * INITIAL_SIZE);
            max_vars = INITIAL_SIZE;
        } else if (num_vars == max_vars) {
            max_vars *= 2;
            global_vars = realloc(global_vars,
                                  sizeof(struct GlobalVariable) * max_vars);
        }

        if (global_vars == NULL) {
            error(-1, "%s", "Allocation failed!");
        }

        //TODO: look for an earlier, deleted slot first?

        /* TODO: This is now actually malloc()'d - don't forget to free. */
        global_vars[num_vars].name = strndup(name, strlen(name));
        global_vars[num_vars].ref = -1;
        return &global_vars[num_vars++].ref;
    } else {
        error(-1, "Could not retrieve variable `%s`", name);
    }
}

/*! Delete the global variable with name `name`. Error if no such variable
    exists. */
void delete_global_variable(char *name) {
    for (int i = 0; i < num_vars; i++) {
        if (strcmp(name, global_vars[i].name) == 0) {
            // Remove the variable by sliding the whole array down
            if (i != num_vars - 1) {
                global_vars[i].name = global_vars[i + 1].name;
                global_vars[i].ref = global_vars[i + 1].ref;
            } else {
                global_vars[i].name = NULL;
                global_vars[i].ref = -1;
            }

            num_vars--;
            //TODO resize array if too small
            return;
        }
    }

    error(-1, "Could not delete variable `%s`", name);
}

/*! Returns true if two keys are equal. Only works on strings and floats. */
bool key_equals(RefId a, RefId b) {
    Reference *ra = deref(a), *rb = deref(b);

    if (ra->type != rb->type) {
        return false;
    }

    switch (ra->type) {
        case VAL_FLOAT:
            return *ra->float_value == *rb->float_value;
        case VAL_STRING:
            return strcmp(ra->string_value, rb->string_value) == 0;
        case VAL_LIST:
        case VAL_DICT:
            error(-1, "%s", "Dict and List types are not valid key types.");
        case VAL_EMPTY:
        default:
            UNREACHABLE();
    }
}

/*! Dereferences a RefId into a Reference* pointer so its type and data can
    be inspected. */
Reference *deref(RefId id) {
    return &(ref_table[id]);
}

/*! ListNode allocation helper. */
ListNode *alloc_list_node(ListNode *next, RefId value) {
    ListNode *l = myalloc(sizeof(ListNode), value);
    l->next = next;
    l->value = value;
    return l;
}

/*! DictNode allocation helper. */
DictNode *alloc_dict_node(DictNode *next,
                                 RefId key, RefId value) {
    DictNode *d = myalloc(sizeof(DictNode), value);
    d->next = next;
    d->key = key;
    d->value = value;
    return d;
}

/*! Allocates an empty reference in the ref_table. */
RefId make_reference() {
    // Allocate a new entry in the reference table, return its refId.
    // set the new ref's type to VAL_EMPTY for sanity.
    if (ref_table == NULL) {
        ref_table = malloc(sizeof(struct Reference) * INITIAL_SIZE);
        max_refs = INITIAL_SIZE;
    } else if (num_refs == max_refs) {
        max_refs *= 2;
        ref_table = realloc(ref_table, sizeof(struct Reference) * max_refs);
    }

    //TODO: search for an empty ref, first? (i.e. those which have
    // been sweeped!!)

    if (ref_table == NULL) {
        error(-1, "%s", "Allocation failed!");
    }

    ref_table[num_refs].type = VAL_EMPTY;
    return num_refs++;
}

/*! Assigns a float to a new reference in the ref_table. */
RefId make_reference_float(float f) {
    RefId r = make_reference();
    deref(r)->type = VAL_FLOAT;
    deref(r)->float_value = myalloc(sizeof(float), r);
    *deref(r)->float_value = f;
    return r;
}

/*! Assigns a string to a new reference in the ref_table. */
RefId make_reference_string(char *c) {
    RefId r = make_reference();
    deref(r)->type = VAL_STRING;
    deref(r)->string_value = eval_string_dup(c, r);
    return r;
}

/*! Assigns a ListNode* to a new reference in the ref_table. */
RefId make_reference_list(ListNode *l) {
    RefId r = make_reference();
    deref(r)->type = VAL_LIST;
    deref(r)->list = l;
    return r;
}

/*! Assigns a DictNode* to a new reference in the ref_table. */
RefId make_reference_dict(DictNode *d) {
    RefId r = make_reference();
    deref(r)->type = VAL_DICT;
    deref(r)->dict = d;
    return r;
}

/*! Clones a key for a dictionary, so keys aren't accidentally bound to each
    other. */
RefId key_clone(RefId ref) {
    // Clone any non-deep type, float and string (that's it...) which are the
    // current key types, as well...
    switch (deref(ref)->type) {
        case VAL_FLOAT:
            return make_reference_float(*deref(ref)->float_value);
        case VAL_STRING:
            return make_reference_string(deref(ref)->string_value);
        default:
            error(-1, "%s", "Only numerical (floats) and string types are "
                            "supported as keys!");
            return 0;
    }
}

/*! Duplicates a string using evaluation-time (student) memory management. */
char *eval_string_dup(char *c, RefId r) {
    // Duplicate the string, allocating the new string onto student memory.
    size_t len = strlen(c);
    char *new_str = myalloc(len + 1, r);
    memcpy(new_str, c, len);
    new_str[len] = '\0';
    return new_str;
}
