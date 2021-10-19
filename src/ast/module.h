
#ifndef _EOKAS_AST_MODULE_H_
#define _EOKAS_AST_MODULE_H_

#include "header.h"

namespace eokas
{
	class ast_module_t
	{
	public:
		ast_module_t();
		virtual ~ast_module_t();

	public:
	    ast_type_ref_t* create_type_ref(ast_node_t* parent, const String& name);
		ast_expr_trinary_t* create_expr_trinary(ast_node_t* parent, ast_expr_t* cond, ast_expr_t* trueV, ast_expr_t* falseV);
		ast_expr_binary_type_t* create_expr_binary_type(ast_node_t* parent, ast_binary_oper_t op, ast_expr_t* lhs, ast_type_t* rhs);
		ast_expr_binary_value_t* create_expr_binary_value(ast_node_t* parent, ast_binary_oper_t op, ast_expr_t* lhs, ast_expr_t* rhs);
		ast_expr_unary_t* create_expr_unary(ast_node_t* parent, ast_unary_oper_t op, ast_expr_t* rhs);
		ast_expr_int_t* create_expr_int(ast_node_t* parent, i64_t value);
		ast_expr_float_t* create_expr_float(ast_node_t* parent, f64_t value);
		ast_expr_bool_t* create_expr_bool(ast_node_t* parent, bool value);
		ast_expr_string_t* create_expr_string(ast_node_t* parent, const String& value);
		ast_expr_symbol_ref_t* create_expr_symbol_ref(ast_node_t* parent, const String& name);
		ast_expr_func_def_t* create_expr_func_def(ast_node_t* parent);
		ast_expr_func_ref_t* create_expr_func_ref(ast_node_t* parent, ast_expr_t* func, const std::vector<ast_expr_t*>& args);
		ast_expr_array_def_t* create_expr_array_def(ast_node_t* parent, const std::vector<ast_expr_t*>& items);
		ast_expr_index_ref_t* create_expr_index_ref(ast_node_t* parent, ast_expr_t* obj, ast_expr_t* key);
		ast_expr_object_def_t* create_expr_object_def(ast_node_t* parent, ast_type_t* type, const std::map<String, ast_expr_t*>& members);
		ast_expr_object_ref_t* create_expr_object_ref(ast_node_t* parent, ast_expr_t* obj, const String& key);
		ast_expr_module_ref_t* create_expr_module_ref(ast_node_t* parent, ast_expr_t* name);
		
	private:
		std::vector<ast_node_t*> nodes;
		ast_expr_func_def_t* func;
		ast_scope_t* scope;
	};
}

#endif //_EOKAS_AST_MODULE_H_
