
#ifndef _EOKAS_AST_HEADER_H_
#define _EOKAS_AST_HEADER_H_

#include <libarchaism/archaism.h>

namespace eokas
{
	enum class ast_node_category_t
	{
		none,
		
		module,
		
		type_ref,
		type_array,
		type_generic,
		
		expr_trinary,
		expr_binary_value,
		expr_binary_type,
		expr_unary,
		expr_int,
		expr_float,
		expr_bool,
		expr_string,
		expr_symbol_ref,
		
		expr_func_def,
		expr_func_ref,
		
		expr_array_def,
		
		expr_object_def,
		expr_object_ref,
		
		expr_index_ref,
		expr_module_ref,
		
		stmt_struct_def,
		stmt_struct_member,
		stmt_enum_def,
		stmt_proc_def,
		stmt_symbol_def,
		stmt_break,
		stmt_continue,
		stmt_return,
		stmt_if,
		stmt_loop,
		stmt_block,
		stmt_assign,
		stmt_call,
	};
	
	enum class ast_binary_oper_t
	{
		Or = 100,
		And = 200,
		Equal = 300, NEqual, LEqual, GEqual, Less, Greater,
		Add = 400, Sub,
		Mul = 500, Div, Mod,
		BitAnd = 600, BitOr, BitXor, ShiftL, ShiftR,
		Is = 700, As,
		MaxPriority = 800,
		Unknown = 0x7FFFFFFF
	};
	
	enum class ast_unary_oper_t
	{
		Pos = 900, Neg, Flip, SizeOf, TypeOf,
		Not = 1000,
		MaxPriority = 1100,
		Unknown = 0x7FFFFFFF
	};
	
	struct ast_pos_t
	{
		int row;
		int col;
		
		ast_pos_t() : row(0), col(0)
		{ }
		
		void set(int r, int c)
		{
			this->row = r;
			this->col = c;
		}
	};
	
	struct ast_node_t
	{
		ast_node_category_t category;
		ast_node_t* parent;
		
		ast_node_t(ast_node_category_t category, ast_node_t* parent)
			: category(category), parent(parent)
		{ }
		
		virtual ~ast_node_t()
		{
			this->category = ast_node_category_t::none;
			this->parent = nullptr;
		}
	};
	
	struct ast_factory_t;
	struct ast_scope_t;
	
	struct ast_type_t;
	struct ast_type_ref_t;
	struct ast_type_array_t;
	struct ast_type_generic_t;
	
	struct ast_expr_t;
	struct ast_expr_trinary_t;
	struct ast_expr_binary_t;
	struct ast_expr_binary_type_t;
	struct ast_expr_binary_value_t;
	struct ast_expr_unary_t;
	struct ast_expr_int_t;
	struct ast_expr_float_t;
	struct ast_expr_bool_t;
	struct ast_expr_string_t;
	struct ast_expr_symbol_ref_t;
	struct ast_expr_func_def_t;
	struct ast_expr_func_ref_t;
	struct ast_expr_array_def_t;
	struct ast_expr_index_ref_t;
	struct ast_expr_object_def_t;
	struct ast_expr_object_ref_t;
	struct ast_expr_module_ref_t;
	
	struct ast_stmt_t;
	struct ast_stmt_struct_member_t;
	struct ast_stmt_struct_def_t;
	struct ast_stmt_enum_def_t;
	struct ast_stmt_proc_def_t;
	struct ast_stmt_symbol_def_t;
	struct ast_stmt_break_t;
	struct ast_stmt_continue_t;
	struct ast_stmt_return_t;
	struct ast_stmt_if_t;
	struct ast_stmt_loop_t;
	struct ast_stmt_block_t;
	struct ast_stmt_call_t;
	struct ast_stmt_assign_t;
	
}

#endif //_EOKAS_AST_HEADER_H_
