
#include "expr.h"

namespace eokas
{
    ast_expr_t::ast_expr_t(ast_node_category_t category, ast_node_t *parent)
        : ast_node_t(category, parent)
        , type(nullptr)
    { }

    ast_expr_t::~ast_expr_t()
    {
        this->type = nullptr;
    }
}