#include <malloc.h>

#include "alloc.h"
#include "types.h"

UnaryExpression *make_unary_expr(Primary *primary, bool is_negated) {
    UnaryExpression *expr = (UnaryExpression *) malloc(sizeof(UnaryExpression));
    expr->primary = primary;
    expr->is_negated = is_negated;
    return expr;
}

MultiplicativeExpression *make_multiplicative_expr(MultiplicativeExpression *left,
                                                   MultiplicativeOperation op,
                                                   UnaryExpression *right) {
    MultiplicativeExpression *expr =
        (MultiplicativeExpression *) malloc(sizeof(MultiplicativeExpression));
    expr->left_expr = left;
    expr->op = op;
    expr->right_expr = right;
    return expr;
}

AdditiveSubexpression *make_additive_expr(AdditiveSubexpression *left,
                                          AdditiveOperation op,
                                          MultiplicativeExpression *right) {
    AdditiveSubexpression *expr =
        (AdditiveSubexpression *) malloc(sizeof(AdditiveSubexpression));
    expr->left_expr = left;
    expr->op = op;
    expr->right_expr = right;
    return expr;
}

Expression *make_expression(AdditiveSubexpression *add_expr) {
    Expression *expr = (Expression *) malloc(sizeof(Expression));
    expr->a_expr = add_expr;
    return expr;
}

Literal *make_literal(LiteralType type, void *ptr) {
    Literal *literal = (Literal *) malloc(sizeof(Literal));
    literal->type = type;
    switch (type) {
        case T_String:
            literal->string = (char *) ptr;
            break;
        case T_Int:
            literal->value = (int) ptr;
            break;
        default:
            return NULL;
    }
    return literal;
}

Dictionary *make_dictionary(Expression **keys, Expression **values, int len) {
    Dictionary *dict = (Dictionary *) malloc(sizeof(Dictionary));
    dict->keys = keys;
    dict->values = values;
    dict->length = len;
    return dict;
}

Enclosure *make_enclosure(EnclosureType type, void *ptr, int length) {
    Enclosure *encl = (Enclosure *) malloc(sizeof(Enclosure));
    encl->type = type;
    encl->length = length;
    switch (type) {
        case T_ParentheticalForm:
        case T_List:
            encl->list = (Expression **) ptr;
            break;
        case T_Dict:
            encl->dict = (Dictionary *) ptr;
            break;
        default:
            return NULL;
    }
    return encl;
}

Atom *make_atom(AtomType type, void *ptr) {
    Atom *atom = (Atom *) malloc(sizeof(Atom));
    atom->type = type;
    switch (type) {
        case T_Literal:
            atom->literal = (Literal *) ptr;
            break;
        case T_Enclosure:
            atom->enclosure = (Enclosure *) ptr;
            break;
        default:
            return NULL;
    }
    return atom;
}

Primary *make_primary(bool is_atom, void *ptr) {
    Primary *primary = (Primary *) malloc(sizeof(Primary));
    primary->is_atom = is_atom;
    if (is_atom) {
        primary->atom = (Atom *) ptr;
    } else {
        primary->target = (Target *) ptr;
    }
    return primary;
}

AttributeReference *make_attributeref(Primary *primary, char *ident) {
    AttributeReference *att_ref =
        (AttributeReference *) malloc(sizeof(AttributeReference));
    att_ref->primary = primary;
    att_ref->identifier = ident;
    return att_ref;
}

Subscription *make_subscription(Primary *primary, Expression **list, int len) {
    Subscription *sub = (Subscription *) malloc(sizeof(Subscription));
    sub->primary = primary;
    sub->expression_list = list;
    sub->list_length = len;
    return sub;
}

Target *make_target(TargetType type, void *ptr) {
    Target *target = (Target *) malloc(sizeof(Target));
    target->type = type;
    switch (type) {
        case T_Identifier:
            target->identifier = (char *) ptr;
            break;
        case T_AttributeReference:
            target->attributeref = (AttributeReference *) ptr;
            break;
        case T_Subscription:
            target->subscription = (Subscription *) ptr;
            break;
        default:
            return NULL;
    }
    return target;
}

DelStatement *make_del_statement(Target **target_list, int length) {
    DelStatement *del_stmt = (DelStatement *) malloc(sizeof(DelStatement));
    del_stmt->target_list = target_list;
    del_stmt->length = length;
    return del_stmt;
}
