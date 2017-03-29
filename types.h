/*! \file
 * This file contains the important type-definitions for the CS24 Python
 * interpreter.
 */

#include <stdbool.h>


/*
 * TODO: This file's definitions are currently in an aesthetically displeasing
 * order. This should be fixed.
 */

typedef struct Primary Primary;
typedef struct MultiplicativeExpression MultiplicativeExpression;
typedef struct AdditiveSubexpression AdditiveSubexpression;
typedef struct Target Target;
typedef struct AssignmentStatement AssignmentStatement;
typedef struct AugmentedAssignment AugmentedAssignment;
typedef struct DelStatement DelStatement;


/*!
 * A unary expression is the simplest type of expression. They consist of a
 * Primary and an optional sign. The sign is ignored unless the Primary is
 * a numeric type (in our case, an integer), in which case it is applied to
 * that primary.
 */
typedef struct UnaryExpression {
    /*! The expression itself. */
    Primary *primary;

    /*! If true, negates the value of the Primary, which must be numeric. */
    bool is_negated;
} UnaryExpression;

/*!
 * Identifies whether two expressions are to be multiplied or divided,
 * or whether just one of the two expressions should be evaluated.
 */
typedef enum MultiplicativeOperation {
    /*!
     * Indicates that the left expression in a given MultiplicativeExpression
     * is to be ignored; that is, the MultiplicativeExpression is to be treated
     * like a UnaryExpression.
     */
    T_Identity,

    /*!
     * Indicates that the left and right subexpressions in a
     * MultiplicativeExpression should be multiplied together.
     */
    T_Multiply,

    /*!
     * Indicates that the left subexpression should be divided by the right
     * subexpression in a given MultiplicativeExpression.
     */
    T_Divide
} MultiplicativeOperation;

/*!
 * Represents either a UnaryExpression or an expression of the forms "a * b"
 * or "a / b", depending on the value of MultiplicativeOperation.
 */
typedef struct MultiplicativeExpression {
    /*!
     * If T_Identity, this expression is to be treated as just its right
     * subexpression; that is, the left subexpression is ignored. Otherwise,
     * indicates whether the subexpressions should be multiplied or divided.
     */
    MultiplicativeOperation op;

    /*! The left subexpression. It is ignored if `op` is T_Identity. */
    MultiplicativeExpression *left_expr;

    /*! The right subexpression. Must be unary thanks to order of operations. */
    UnaryExpression *right_expr;
} MultiplicativeExpression;

/*!
 * Identifies whether two expressions are to be added together or subtracted,
 * or if just one of the two expressions should be evaluated.
 */
typedef enum AdditiveOperation {
    /*!
     * Indicates that the left expression in a given AdditiveSubexpression
     * is to be ignored; that is, the AdditiveSubexpression is to be treated
     * like a MultiplicativeExpression.
     */
    T_Zero,
    T_Add,
    T_Subtract
} AdditiveOperation;

/*!
 * Represents either a MultiplicativeExpression or an expression of the form
 * "a + b" or "a - b", where `a` is an AdditiveSubexpression and `b` is a
 * MultiplicativeExpression, depending on its operation.
 */
typedef struct AdditiveSubexpression {
    /*!
     * If T_Zero, this expression is to be treated as just its right
     * subexpression; that is, left_expr is ignored. Otherwise, Indicates
     * whether left_expr and right_expr are to be added or subtracted.
     */
    AdditiveOperation op;

    /*! The left subexpression. It is ignored if `op` is T_Zero. */
    AdditiveSubexpression *left_expr;

    /*!
     * The right subexpression. It must be multiplicative thanks to order of
     * operations.
     */
    MultiplicativeExpression *right_expr;
} AdditiveSubexpression;

/*!
 * Represents an expression that is one of the following:
 *     - A series of additions, subtractions, multiplications, and divisions
 *       of numeric or string types (though note that strings don't support
 *       subtraction or division)
 *     - A primary not in the above category
 */
typedef struct Expression {
    AdditiveSubexpression *a_expr;
} Expression;


/*!
 * Indicates whether a literal is a string or an integer.
 */
typedef enum LiteralType {
    T_String,
    T_Int
    /* TODO: support longs and floats */
} LiteralType;

/*
 * A literal is a string or an integer. In the future, other numeric types
 * such as longs, floats, or imaginary numbers could be supported here as well.
 */
typedef struct Literal {
    LiteralType type;

    union {
        char *string;
        int value;
    };
} Literal;


/*!
 * A dictionary consists of a list of expressions called `keys` and a list of
 * expressions called `values`. The nth key is associated with the nth value.
 * Thus, these two lists must have the same length.
 *
 * Notably, keys cannot be mutable types, e.g. lists or other dictionaries.
 * This is not enforced by the typing system, but by the evaluator.
 */
typedef struct Dictionary {
    Expression **keys;
    Expression **values;

    int length;
} Dictionary;

/*!
 * Indicates whether an enclosure is a parenthesized form, a list, or a
 * dictionary.
 */
typedef enum EnclosureType {
    T_ParentheticalForm,
    T_List,
    T_Dict
} EnclosureType;

/*!
 * An enclosure is a parenthesized form, a list, or a dictionary.
 * Parenthesized forms and lists are just lists of expressions, with the
 * major difference that lists are mutable, while parenthetical forms are not.
 * Parenthesized forms of length 2 or greater are known as tuples.
 */
typedef struct Enclosure {
    EnclosureType type;

    union {
        Expression **parenth_form;
        Expression **list;
        Dictionary dict;
    };

    /*!
     * Indicates the length of the enclosure. Note that this attribute is
     * redundant if the enclosure is a dictionary, as dictionaries also keep
     * track of their own lengths.
     */
    int length;
} Enclosure;

/*!
 * Indicates whether the atom is a literal or an enclosure.
 */
typedef enum AtomType {
    T_Literal,
    T_Enclosure
} AtomType;

/*!
 * An atom is a literal or an enclosure. It is very important to note that this
 * is not the same definition as is given in the Python Language Reference;
 * here, atoms are not allowed to be identifiers. This is because we do not
 * differentiate between targets that are identifiers and atoms that are
 * identifiers.
 */
typedef struct Atom {
    AtomType type;
    union {
        Literal *literal;
        Enclosure *enclosure;
    };
} Atom;

/*!
 * A primary is an atom (i.e. a literal or an enclosure) or a target (i.e. an
 * identifier, an attribute reference, or a subscription).
 */
typedef struct Primary {
    bool is_atom;

    union {
        Atom *atom;
        Target *target;
    };
} Primary;

/*!
 * An attribute reference is a primary followed by a period and a name, e.g.
 * "elements.size".
 */
typedef struct AttributeReference {
    Primary *primary;
    char *identifier;
} AttributeReference;

/*!
 * A subscription selects an item of a sequence (string, tuple, or list) or
 * mapping (dictionary) object. It consists of a primary, from which the item
 * is selected, and a list of expressions, which indicates which item is to be
 * picked.
 *
 * If the primary is a mapping, the expression list must evaluate to an
 * Expression which is one of the keys of the mapping. If the primary is a
 * sequence, the expression list must be a plain integer. If the value is
 * negative, the length of the sequence is added to it. Then, that element
 * of the sequence is chosen.
 */
typedef struct Subscription {
    Primary *primary;
    Expression **expression_list;
    int list_length;
} Subscription;

/*!
 * Indicates whether a target is an identifier, an attribute reference, or
 * a subscription.
 */
typedef enum TargetType {
    T_Identifier,
    T_AttributeReference,
    T_Subscription,
    /* TODO: support slicings */
} TargetType;

/*!
 * A target is an identifier, an attribute reference, or a subscription,
 * and is used by assignment statements (augmented and not) to indicate
 * variable bindings. That is, the left-hand side of an assignment statement
 * is a target.
 *
 * Notably, `target` is a misnomer. Anything that is an identifier must be
 * called a target; however, there are many cases where identifiers are not
 * actually targets of anything. See the comment above `Primary`.
 */
typedef struct Target {
    TargetType type;
    union {
        char *identifier;
        AttributeReference attributeref;
        Subscription subscription;
    };
} Target;

/*!
 * An assignment statement is used to (re)bind names to values and to
 * modify attributes or items of mutable objects. For example, the statement
 * "a, b = 2, 4" is an assignment statement. Additionally, the statement
 * "a, b, c = d, e, f = 1, 2, 3" is another example.
 *
 * Note that anything on the left hand side can  be just one element (in which
 * case the right hand side must also be just one element), or it can be
 * an iterable of some sort, in which case the right hand side must be
 * an iterable of the same length.
 */
typedef struct AssignmentStatement {
    /*!
     * The list of target lists to which values will be assigned. Each element
     * in this list is a list of targets. For example, in the assignment
     * "a, b = c, d = 1, 2", the target lists would be ['a', 'b'] and ['c',
     * 'd'].
     */
    Target ***target_list_list;

    /*!
     * The list of expressions which will be assigned to the targets. For
     * example, in the assignment "a, b = c, d = 1, 2", the expression list
     * would be [1, 2].
     */
    Expression **expression_list;

    /*!
     * The length of the list of expressions. For example, in the
     * assignment "a, b, c = d, e, f = 1, 2, 3", this value would be 3.
     * Note that this must match the number of targets per list of targets.
     */
    int num_expressions;

    /*!
     * The number of lists of targets. For example, in the assignment
     * "a, b, c = d, e, f = 1, 2, 3", this value would be 2.
     */
    int num_target_lists;
} AssignmentStatement;

/*!
 * Indicates whether the augmented assignment statement uses the +=, -=, *=,
 * or /= operator.
 */
typedef enum AugmentedOperation {
    T_PlusEquals,
    T_MinusEquals,
    T_TimesEquals,
    T_DivideEquals
} AugmentedOperation;

/*!
 * Indicates exactly those statements of the form "a OP b", where `a` is a
 * target and `b` is an expression, `OP` being an augmented assignment operation
 * of some sort.
 */
typedef struct AugmentedAssignment {
    Target *target;
    AugmentedOperation op;
    Expression *expression;
} AugmentedAssignment;

/*!
 * A del statement indicates a list of targets that must be unbound from their
 * identifiers, aka "deleted".
 */
typedef struct DelStatement {
    Target **target_list;
} DelStatement;

/*!
 * Indicates whether a statement is an assignment statement, an augmented
 * assignment statement, or a "del" statement.
 */
typedef enum StatementType {
    T_AssignmentStatement,
    T_AugmentedAssignment,
    T_DelStatement
} StatementType;

/*!
 * Statements represent a single logical line in Python code. In our case, they
 * must be one of the following:
 *     - An assignment statement (e.g. "a = 2")
 *     - An augmented assignment statement (e.g. "a += 2")
 *     - A del statement (e.g. "del a")
 *
 * Of course, Python natively supports more types of statements than this.
 * (For more info, see https://docs.python.org/2/reference/simple_stmts.html.)
 *
 * This allows multiple assignment, i.e. "a, b = 2, a" is allowed.
 */
typedef struct Statement {
    StatementType type;

    /*! The statement itself. */
    union {
        AssignmentStatement assign_stmt;
        AugmentedAssignment augmented_stmt;
        DelStatement del_stmt;
    };
} Statement;
