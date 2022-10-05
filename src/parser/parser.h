#ifndef _EOKAS_PARSER_H_
#define _EOKAS_PARSER_H_

#include <eokas-base/main.h>
#include "scanner.h"
#include "../ast/ast.h"

namespace eokas
{
	class parser_t
	{
	public:
		parser_t();
		
		~parser_t();
	
	public:
		ast_node_module_t* parse(const char* source);
		void clear();
		
		ast_node_module_t* parse_module();
		ast_node_import_t* parse_import(ast_node_t* p);
		ast_node_export_t* parse_export(ast_node_t* p);
		
		ast_node_type_t* parse_type(ast_node_t* p);
		
		ast_node_expr_t* parse_expr(ast_node_t* p);
		ast_node_expr_t* parse_expr_trinary(ast_node_t* p);
		ast_node_expr_t* parse_expr_binary(ast_node_t* p, int priority = 1);
		ast_node_expr_t* parse_expr_unary(ast_node_t* p);
		ast_node_expr_t* parse_expr_suffixed(ast_node_t* p);
		ast_node_expr_t* parse_expr_primary(ast_node_t* p);
		ast_node_expr_t* parse_literal_int(ast_node_t* p);
		ast_node_expr_t* parse_literal_float(ast_node_t* p);
		ast_node_expr_t* parse_literal_bool(ast_node_t* p);
		ast_node_expr_t* parse_literal_string(ast_node_t* p);
		ast_node_expr_t* parse_func_def(ast_node_t* p);
		bool parse_func_params(ast_node_func_def_t* node);
		bool parse_func_body(ast_node_func_def_t* node);
		ast_node_expr_t* parse_object_def(ast_node_t* p);
		ast_node_expr_t* parse_func_call(ast_node_t* p, ast_node_expr_t* primary);
		ast_node_expr_t* parse_array_def(ast_node_t* p);
		ast_node_expr_t* parse_index_ref(ast_node_t* p, ast_node_expr_t* primary);
		bool parse_object_field(ast_node_object_def_t* node);
		ast_node_expr_t* parse_object_ref(ast_node_t* p, ast_node_expr_t* primary);
		
		ast_node_stmt_t* parse_stmt(ast_node_t* p);
		ast_node_struct_def_t* parse_stmt_struct_def(ast_node_t* p);
		bool parse_stmt_struct_member(ast_node_struct_def_t* node);
		ast_node_enum_def_t* parse_stmt_enum_def(ast_node_t* p);
		ast_node_proc_def_t* parse_stmt_proc_def(ast_node_t* p);
		ast_node_symbol_def_t* parse_stmt_symbol_def(ast_node_t* p);
		ast_node_continue_t* parse_stmt_continue(ast_node_t* p);
		ast_node_break_t* parse_stmt_break(ast_node_t* p);
		ast_node_return_t* parse_stmt_return(ast_node_t* p);
		ast_node_if_t* parse_stmt_if(ast_node_t* p);
		ast_node_loop_t* parse_stmt_loop(ast_node_t* p);
		ast_node_stmt_t* parse_stmt_loop_init(ast_node_t* p);
		ast_node_expr_t* parse_stmt_loop_cond(ast_node_t* p);
		ast_node_stmt_t* parse_stmt_loop_step(ast_node_t* p);
		ast_node_block_t* parse_stmt_block(ast_node_t* p);
		ast_node_stmt_t* parse_stmt_assign_or_call(ast_node_t* p);
		
		void next_token();
		struct token_t& token();
		struct token_t& look_ahead_token();
		bool check_token(const token_t::token_type& tokenType, bool required = true, bool movenext = true);
		ast_unary_oper_t check_unary_oper(bool required = true, bool movenext = true);
		ast_binary_oper_t check_binary_oper(int priority, bool required = true, bool movenext = true);
		
		void error(const char* fmt, ...);
		void error_token_unexpected();
		
		const String& error() const;
	
	private:
		class scanner_t* scanner;
		class ast_factory_t* factory;
		String errormsg;
	};
}

#endif//_EOKAS_PARSER_H_
