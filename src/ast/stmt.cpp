
#include "stmt.h"

namespace eokas
{
    ast_stmt_t::ast_stmt_t(ast_node_category_t category, ast_node_t* parent)
        : ast_node_t(category, parent)
    { }
}
