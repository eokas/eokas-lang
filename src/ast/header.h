
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
		
		stmt_schema_def,
		stmt_schema_member,
		stmt_struct_def,
		stmt_struct_member,
		stmt_proc_def,
		stmt_symbol_def,
		stmt_break,
		stmt_continue,
		stmt_return,
		stmt_if,
		stmt_while,
		stmt_for,
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
	
	class ast_scope_t;
	class ast_type_t;
	class ast_expr_t;
	class ast_symbol_t;
	class ast_func_t;
}

#endif //_EOKAS_AST_HEADER_H_
