
#include "type.h"

namespace eokas
{
    ast_type_t::ast_type_t(ast_node_category_t category, ast_node_t* parent)
        : ast_node_t(category, parent)
    { }
	
	ast_type_ref_t::ast_type_ref_t(ast_node_t* parent)
		: ast_type_t(ast_node_category_t::type_ref, parent)
		, name("")
	{ }
	
	ast_type_array_t::ast_type_array_t(ast_node_t* parent)
		: ast_type_t(ast_node_category_t::type_array, parent)
		, elementType(nullptr)
		, length(0)
	{ }
	
	ast_type_generic_t::ast_type_generic_t(ast_node_t* parent)
		: ast_type_t(ast_node_category_t::type_generic, parent)
		, name("")
		, args()
	{ }
}
