
#include "stmt.h"

namespace eokas
{
    ast_stmt_t::ast_stmt_t(ast_node_category_t category, ast_node_t* parent)
        : ast_node_t(category, parent)
    { }
	
	ast_stmt_schema_member_t::ast_stmt_schema_member_t(ast_node_t* parent)
		: ast_stmt_t(ast_node_category_t::stmt_schema_member, parent)
		, name("")
		, type(nullptr)
	{ }
	
	ast_stmt_schema_def_t::ast_stmt_schema_def_t(ast_node_t* parent)
		: ast_stmt_t(ast_node_category_t::stmt_schema_def, parent)
		, name("")
		, schema(nullptr)
		, members()
	{ }
	
	ast_stmt_struct_member_t::ast_stmt_struct_member_t(ast_node_t* parent)
		: ast_stmt_t(ast_node_category_t::stmt_struct_member, parent)
		, name("")
		, type(nullptr)
		, value(nullptr)
	{ }
	
	ast_stmt_struct_def_t::ast_stmt_struct_def_t(ast_node_t* parent)
		: ast_stmt_t(ast_node_category_t::stmt_struct_def, parent)
		, name("")
		, schema(nullptr)
		, members()
	{ }
	
	ast_stmt_proc_def_t::ast_stmt_proc_def_t(ast_node_t* parent)
		: ast_stmt_t(ast_node_category_t::stmt_proc_def, parent)
		, name("")
		, type(nullptr)
		, args()
	{ }
	
	ast_stmt_symbol_def_t::ast_stmt_symbol_def_t(ast_node_t* parent)
		: ast_stmt_t(ast_node_category_t::stmt_symbol_def, parent)
		, name("")
		, type(nullptr)
		, value(nullptr)
		, variable(false)
	{ }
}
