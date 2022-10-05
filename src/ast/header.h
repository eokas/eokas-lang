#ifndef _EOKAS_AST_HEADER_H_
#define _EOKAS_AST_HEADER_H_

#include <eokas-base/main.h>

namespace eokas
{
	enum class ast_category_t
	{
		NONE,

		MODULE, IMPORT, EXPORT,

		TYPE,
		
        FUNC_DEF, FUNC_REF,
        SYMBOL_DEF, SYMBOL_REF,

        EXPR_TRINARY, EXPR_BINARY, EXPR_UNARY,
        LITERAL_INT, LITERAL_FLOAT, LITERAL_BOOL, LITERAL_STRING,
        ARRAY_DEF, ARRAY_REF,
		OBJECT_DEF, OBJECT_REF,

		STRUCT_DEF, ENUM_DEF, PROC_DEF,

        RETURN, IF, LOOP, BREAK, CONTINUE, BLOCK, ASSIGN, INVOKE,
	};
	
	enum class ast_binary_oper_t
	{
		OR = 100,
        AND = 200,
        EQ = 300, NE, LE, GE, LT, GT,
        ADD = 400, SUB,
        MUL = 500, DIV, MOD,
        BIT_AND = 600, BIT_OR, BIT_XOR, SHIFT_L, SHIFT_R,
        MAX_PRIORITY = 800,
        UNKNOWN = 0x7FFFFFFF
	};
	
	enum class ast_unary_oper_t
	{
		POS = 900, NEG, FLIP, SIZE_OF, TYPE_OF,
        NOT = 1000,
        MAX_PRIORITY = 1100,
        UNKNOWN = 0x7FFFFFFF
	};
	
	struct ast_pos_t
	{
		int row;
		int col;
		
		ast_pos_t()
			: row(0), col(0)
		{
		}
		
		void set(int r, int c)
		{
			this->row = r;
			this->col = c;
		}
	};
	
    struct ast_node_t;

    struct ast_node_module_t;
	struct ast_node_import_t;
    struct ast_node_export_t;

	struct ast_node_type_t;
	struct ast_node_expr_t;
	struct ast_node_stmt_t;

    struct ast_node_func_def_t;
    struct ast_node_func_ref_t;

    struct ast_node_symbol_def_t;
    struct ast_node_symbol_ref_t;

	struct ast_node_expr_trinary_t;
	struct ast_node_expr_binary_t;
	struct ast_node_expr_unary_t;

	struct ast_node_literal_int_t;
	struct ast_node_literal_float_t;
	struct ast_node_literal_bool_t;
	struct ast_node_literal_string_t;

	struct ast_node_array_def_t;
	struct ast_node_array_ref_t;
	struct ast_node_object_def_t;
	struct ast_node_object_ref_t;

	struct ast_node_struct_def_t;
	struct ast_node_enum_def_t;
	struct ast_node_proc_def_t;

    struct ast_node_return_t;
    struct ast_node_if_t;
    struct ast_node_loop_t;
	struct ast_node_break_t;
	struct ast_node_continue_t;
	struct ast_node_block_t;
	struct ast_node_assign_t;
	struct ast_node_invoke_t;

    struct ast_factory_t;
    struct ast_scope_t;

}

#endif //_EOKAS_AST_HEADER_H_
