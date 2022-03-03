#ifndef _EOKAS_AST_FACTORY_H_
#define _EOKAS_AST_FACTORY_H_

#include "header.h"

namespace eokas
{
	class ast_factory_t
	{
	public:
		ast_factory_t();
		virtual ~ast_factory_t();
	
	public:
		ast_type_ref_t* create_type_ref(ast_node_t* parent);
		ast_type_array_t* create_type_array(ast_node_t* parent);
		ast_type_generic_t* create_type_generic(ast_node_t* parent);
		
		ast_expr_trinary_t* create_expr_trinary(ast_node_t* parent);
		ast_expr_binary_t* create_expr_binary(ast_node_t* parent);
		ast_expr_unary_t* create_expr_unary(ast_node_t* parent);
		ast_expr_int_t* create_expr_int(ast_node_t* parent);
		ast_expr_float_t* create_expr_float(ast_node_t* parent);
		ast_expr_bool_t* create_expr_bool(ast_node_t* parent);
		ast_expr_string_t* create_expr_string(ast_node_t* parent);
		ast_expr_symbol_ref_t* create_expr_symbol_ref(ast_node_t* parent);
		ast_expr_func_def_t* create_expr_func_def(ast_node_t* parent);
		ast_expr_func_ref_t* create_expr_func_ref(ast_node_t* parent);
		ast_expr_array_def_t* create_expr_array_def(ast_node_t* parent);
		ast_expr_index_ref_t* create_expr_index_ref(ast_node_t* parent);
		ast_expr_object_def_t* create_expr_object_def(ast_node_t* parent);
		ast_expr_object_ref_t* create_expr_object_ref(ast_node_t* parent);
		ast_expr_module_ref_t* create_expr_module_ref(ast_node_t* parent);
		
		ast_stmt_struct_member_t* create_stmt_struct_member(ast_node_t* parent);
		ast_stmt_struct_def_t* create_stmt_struct_def(ast_node_t* parent);
		ast_stmt_enum_def_t* create_stmt_enum_def(ast_node_t* parent);
		ast_stmt_proc_def_t* create_stmt_proc_def(ast_node_t* parent);
		ast_stmt_symbol_def_t* create_stmt_symbol_def(ast_node_t* parent);
		ast_stmt_break_t* create_stmt_break(ast_node_t* parent);
		ast_stmt_continue_t* create_stmt_continue(ast_node_t* parent);
		ast_stmt_return_t* create_stmt_return(ast_node_t* parent);
		ast_stmt_if_t* create_stmt_if(ast_node_t* parent);
		ast_stmt_loop_t* create_stmt_loop(ast_node_t* parent);
		ast_stmt_block_t* create_stmt_block(ast_node_t* parent);
		ast_stmt_call_t* create_stmt_call(ast_node_t* parent);
		ast_stmt_assign_t* create_stmt_assign(ast_node_t* parent);
	
	private:
		std::vector<ast_node_t*> nodes;
	};
}

#endif //_EOKAS_AST_FACTORY_H_
