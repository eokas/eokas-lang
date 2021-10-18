
#ifndef _EOKAS_AST_STMT_H_
#define _EOKAS_AST_STMT_H_

#include "header.h"

namespace eokas
{
	struct ast_stmt_t : public ast_node_t
	{
		ast_stmt_t(ast_node_category_t category, ast_node_t* parent);
	};
	
	struct ast_stmt_schema_member_t : public ast_stmt_t
	{
		String name;
		ast_type_t* type;
		
		explicit ast_stmt_schema_member_t(ast_node_t* parent);
	};
	
	struct ast_stmt_schema_def_t : public ast_stmt_t
	{
		String name;
		ast_type_ref_t* schema;
		std::map<String, ast_stmt_schema_member_t*> members;
		
		explicit ast_stmt_schema_def_t(ast_node_t* parent);
	};
	
	struct ast_stmt_struct_member_t : public ast_stmt_t
	{
		String name;
		ast_type_t* type;
		ast_expr_t* value;
		
		explicit ast_stmt_struct_member_t(ast_node_t* parent);
	};
	
	struct ast_stmt_struct_def_t : public ast_stmt_t
	{
		String name;
		ast_type_ref_t* schema;
		std::map<String, ast_stmt_struct_member_t*> members;
		
		explicit ast_stmt_struct_def_t(ast_node_t* parent);
	};
	
	struct ast_stmt_proc_def_t : public ast_stmt_t
	{
		String name;
		ast_type_t* type;
		std::map<String, ast_type_t*> args;
		
		explicit ast_stmt_proc_def_t(ast_node_t* parent);
	};
	
	struct ast_stmt_symbol_def_t : public ast_stmt_t
	{
		String name;
		ast_type_t* type;
		ast_expr_t* value;
		bool variable;
		
		explicit ast_stmt_symbol_def_t(ast_node_t* parent);
	};
	
	struct ast_stmt_break_t : public ast_stmt_t
	{
		explicit ast_stmt_break_t(ast_node_t* parent)
			: ast_stmt_t(ast_node_category_t::stmt_break, parent)
		{ }
	};
	
	struct ast_stmt_continue_t : public ast_stmt_t
	{
		explicit ast_stmt_continue_t(ast_node_t* parent)
			: ast_stmt_t(ast_node_category_t::stmt_continue, parent)
		{ }
	};
	
	struct ast_stmt_return_t : public ast_stmt_t
	{
		ast_expr_t* value;
		
		explicit ast_stmt_return_t(ast_node_t* parent)
			: ast_stmt_t(ast_node_category_t::stmt_return, parent), value(nullptr)
		{ }
	};
	
	struct ast_stmt_if_t : public ast_stmt_t
	{
		ast_expr_t* cond;
		ast_stmt_t* branch_true;
		ast_stmt_t* branch_false;
		
		explicit ast_stmt_if_t(ast_node_t* parent)
			: ast_stmt_t(ast_node_category_t::stmt_if, parent), cond(nullptr), branch_true(nullptr), branch_false(
			nullptr)
		{ }
	};
	
	struct ast_stmt_while_t : public ast_stmt_t
	{
		ast_expr_t* cond;
		ast_stmt_t* body;
		
		explicit ast_stmt_while_t(ast_node_t* parent)
			: ast_stmt_t(ast_node_category_t::stmt_while, parent), cond(nullptr), body(nullptr)
		{ }
	};
	
	struct ast_stmt_for_t : public ast_stmt_t
	{
		ast_stmt_t* init;
		ast_expr_t* cond;
		ast_stmt_t* step;
		ast_stmt_t* body;
		
		explicit ast_stmt_for_t(ast_node_t* parent)
			: ast_stmt_t(ast_node_category_t::stmt_for, parent), init(nullptr), cond(nullptr), step(nullptr), body(
			nullptr)
		{ }
	};
	
	struct ast_stmt_block_t : public ast_stmt_t
	{
		std::vector<ast_stmt_t*> stmts;
		
		explicit ast_stmt_block_t(ast_node_t* parent)
			: ast_stmt_t(ast_node_category_t::stmt_block, parent), stmts()
		{ }
	};
	
	struct ast_stmt_call_t : public ast_stmt_t
	{
		ast_expr_func_ref_t* expr;
		
		explicit ast_stmt_call_t(ast_node_t* parent)
			: ast_stmt_t(ast_node_category_t::stmt_call, parent), expr(nullptr)
		{ }
	};
	
	struct ast_stmt_assign_t : public ast_stmt_t
	{
		ast_expr_t* left;
		ast_expr_t* right;
		
		explicit ast_stmt_assign_t(ast_node_t* parent)
			: ast_stmt_t(ast_node_category_t::stmt_assign, parent), left(nullptr), right(nullptr)
		{ }
	};
}

#endif //_EOKAS_AST_STMT_H_