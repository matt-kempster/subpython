#include "types.h"

DelStatement *make_del_statement(Target **target_list, int length) {
    DelStatement del_stmt = (DelStatement *) malloc(sizeof(DelStatement));
    del_stmt->target_list = target_list;
    del_stmt->length = length;
}
