#include <string.h>

#include "global.h"
#include "eval.h"

/* Global variable information. */

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

void eval_stmt(ParseStatement *stmt) {
    switch (stmt->type) {
      case STMT_DEL: {
        delete_global_variable(stmt->identifier);
      }
      case STMT_EXPR: {
        eval_expr(stmt->expr);
      }
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
                }

                if (node == NULL) {
                    //TODO: error, value can't be found.
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
            }
            return make_reference_list(eval_list);
        }
        case EXPR_DICT: {
            ParseDictNode *parse_dict = expr->dict;
            DictNode *eval_dict = NULL;
            while (parse_dict != NULL) {
                eval_dict = alloc_dict_node(eval_dict,
                                            eval_expr(parse_dict->key),
                                            eval_expr(parse_dict->value));
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
            if (deref(lhs)->type == VAL_DICT) {
                rhs = eval_expr(expr->rhs);
                DictNode *node = deref(lhs)->dict;

                while (node != NULL) {
                    if (key_equals(node->key, rhs)) {
                        break;
                    }
                }

                if (node == NULL) {
                    node = alloc_dict_node(deref(lhs)->dict,
                                           key_clone(rhs),
                                           // Placeholder
                                           make_reference_float(0));
                    deref(lhs)->dict = node;
                }

                return &(node->value);
            } else {
                //TODO: error, no other type can be subscripted as an LVAL
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
        //TODO: Error.
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
            global_vars = realloc(global_vars, sizeof(struct GlobalVariable) * max_vars);
        }

        if (global_vars == NULL) {
            error(-1, "%s", "Allocation failed!");
        }

        global_vars[num_vars].name = eval_string_dup(name);
        // Assign a placeholder for now. If `create == true`, then it will be
        // assigned a real value later, don't worry.
        RefId ref = make_reference_float(0);
        global_vars[num_vars].ref = ref;

        num_vars++;
        //TODO: eww @ -1.
        return &global_vars[num_vars - 1].ref;
    } else {
        error(-1, "Could not retrieve variable `%s`", name);
    }
}

void delete_global_variable(char *name) {
    // Delete the global variable with name `name`. Error if no such variable
    // exists
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

bool key_equals(RefId a, RefId b) {
    Reference *ra = deref(a), *rb = deref(b);

    if (ra->type != rb->type) {
        return false;
    }

    switch (ra->type) {
        case VAL_FLOAT:
            return ra->float_value == rb->float_value;
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

Reference *deref(RefId id) {
    return &(ref_table[id]);
}

ListNode *alloc_list_node(ListNode *next, RefId value) {
    ListNode *l = malloc(sizeof(ListNode));
    l->next = next;
    l->value = value;
    return l;
}

DictNode *alloc_dict_node(DictNode *next,
                                 RefId key, RefId value) {
    //TODO: replace with student allocator.
    DictNode *d = malloc(sizeof(DictNode));
    d->next = next;
    d->key = key;
    d->value = value;
    return d;
}

RefId make_reference() {
    // Allocate a new entry in the reference table, return its refId.
    // set the new ref's type to VAL_EMPTY for sanity.
    if (ref_table == NULL) {
        ref_table = malloc(sizeof(struct Reference) * INITIAL_SIZE);
        max_refs = INITIAL_SIZE;
    } else if (num_refs == max_refs) {
        max_refs *= 2;
        ref_table = realloc(ref_table, sizeof(struct Reference) * max_vars);
    }

    if (ref_table == NULL) {
        error(-1, "%s", "Allocation failed!");
    }

    ref_table[num_refs].type = VAL_EMPTY;
    return num_refs++;
}

RefId make_reference_float(float f) {
    RefId r = make_reference();
    deref(r)->type = VAL_FLOAT;
    //TODO: use student memory
    deref(r)->float_value = malloc(sizeof(float));
    *deref(r)->float_value = f;
    return r;
}

RefId make_reference_string(char *c) {
    RefId r = make_reference();
    deref(r)->type = VAL_STRING;
    deref(r)->string_value = eval_string_dup(c);
    return r;
}

RefId make_reference_list(ListNode *l) {
    RefId r = make_reference();
    deref(r)->type = VAL_LIST;
    deref(r)->list = l;
    return r;
}

RefId make_reference_dict(DictNode *d) {
    RefId r = make_reference();
    deref(r)->type = VAL_DICT;
    deref(r)->dict = d;
    return r;
}

void assign_ref(RefId a, RefId b) {
    // Fully copy the Reference struct (ie type and value).
    memmove(deref(a), deref(b), sizeof(Reference));
}

RefId key_clone(RefId ref) {
    // Clone any non-deep type, float and string (that's it...) which are the
    // current key types, as well...
    switch (deref(ref)->type) {
        case VAL_FLOAT:
            return make_reference_float(*deref(ref)->float_value);
        case VAL_STRING:
            return make_reference_string(deref(ref)->string_value);
        default:
            //TODO error out
            return 0;
    }
}

char *eval_string_dup(char *c) {
    // Duplicate the string, allocating the new string onto student memory.
    size_t len = strlen(c);
    //TODO student memory
    char *new_str = malloc(len + 1);
    memcpy(new_str, c, len);
    new_str[len] = '\0';
    return new_str;
}
