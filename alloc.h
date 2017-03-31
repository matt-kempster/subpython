#ifndef ALLOC_H
#define ALLOC_H

#include "types.h"

UnaryExpression *make_unary_expr(Primary *, bool);
MultiplicativeExpression *make_multiplicative_expr(
    MultiplicativeExpression *, MultiplicativeOperation, UnaryExpression *);
AdditiveSubexpression *make_additive_expr(
    AdditiveSubexpression *, AdditiveOperation, MultiplicativeExpression *);
Expression *make_expression(AdditiveSubexpression *);
Literal *make_literal(LiteralType, void *);
Dictionary *make_dictionary(Expression **, Expression **, int);
Enclosure *make_enclosure(EnclosureType, void *, int);
Atom *make_atom(AtomType, void *);
Primary *make_primary(bool, void *);
AttributeReference *make_attributeref(Primary *, char *);
Subscription *make_subscription(Primary *, Expression **, int);
Target *make_target(TargetType, void *);
DelStatement *make_del_statement(Target **, int);

#endif /* ALLOC_H */
