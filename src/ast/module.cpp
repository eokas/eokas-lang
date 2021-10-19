
#include "module.h"
#include "object.h"
#include "type.h"
#include "expr.h"

namespace eokas
{
	ast_module_t::ast_module_t()
		: nodes()
		, func(nullptr)
		, scope(nullptr)
	{ }
	
	ast_module_t::~ast_module_t()
	{
		_DeleteList(this->nodes);
		this->func = nullptr;
		_DeletePointer(this->scope);
	}
	
	ast_type_ref_t* ast_module_t::create_type_ref(ast_node_t* parent, const String& name)
	{
		auto* node = new ast_type_ref_t(parent);
		node->name = name;
		this->nodes.push_back(node);
		return node;
	}
	
	ast_expr_trinary_t* ast_module_t::create_expr_trinary(ast_node_t* parent, ast_expr_t* cond, ast_expr_t* trueV, ast_expr_t* falseV)
	{
		auto* node = new ast_expr_trinary_t(parent);
		node->cond = cond;
		node->branch_true = trueV;
		node->branch_false = falseV;
		this->nodes.push_back(node);
		return node;
	}
	
	ast_expr_binary_type_t* ast_module_t::create_expr_binary_type(ast_node_t* parent, ast_binary_oper_t op, ast_expr_t* lhs, ast_type_t* rhs)
	{
		auto* node = new ast_expr_binary_type_t(parent);
		node->op = op;
		node->left = lhs;
		node->right = rhs;
		this->nodes.push_back(node);
		return node;
	}
	
	ast_expr_binary_value_t* ast_module_t::create_expr_binary_value(ast_node_t* parent, ast_binary_oper_t op, ast_expr_t* lhs, ast_expr_t* rhs)
	{
		auto* node = new ast_expr_binary_value_t(parent);
		node->op = op;
		node->left = lhs;
		node->right = rhs;
		this->nodes.push_back(node);
		return node;
	}
	
	ast_expr_unary_t* ast_module_t::create_expr_unary(ast_node_t* parent, ast_unary_oper_t op, ast_expr_t* rhs)
	{
		auto* node = new ast_expr_unary_t(parent);
		node->op = op;
		node->right = rhs;
		this->nodes.push_back(node);
		return node;
	}
	
	ast_expr_int_t* ast_module_t::create_expr_int(ast_node_t* parent, i64_t value)
	{
		auto* node = new ast_expr_int_t(parent);
		node->value = value;
		this->nodes.push_back(node);
		return node;
	}
	
	ast_expr_float_t* ast_module_t::create_expr_float(ast_node_t* parent, f64_t value)
	{
		auto* node = new ast_expr_float_t(parent);
		node->value = value;
		this->nodes.push_back(node);
		return node;
	}
	
	ast_expr_bool_t* ast_module_t::create_expr_bool(ast_node_t* parent, bool value)
	{
		auto* node = new ast_expr_bool_t(parent);
		node->value = value;
		this->nodes.push_back(node);
		return node;
	}
	
	ast_expr_string_t* ast_module_t::create_expr_string(ast_node_t* parent, const String& value)
	{
		auto* node = new ast_expr_string_t(parent);
		node->value = value;
		this->nodes.push_back(node);
		return node;
	}
	
	ast_expr_symbol_ref_t* ast_module_t::create_expr_symbol_ref(ast_node_t* parent, const String& name)
	{
		auto* node = new ast_expr_symbol_ref_t(parent);
		node->name = name;
		this->nodes.push_back(node);
		return node;
	}
	
	ast_expr_func_def_t* ast_module_t::create_expr_func_def(ast_node_t* parent)
	{
		auto* node = new ast_expr_func_def_t(parent);
		this->nodes.push_back(node);
		return node;
	}
	
	ast_expr_func_ref_t* ast_module_t::create_expr_func_ref(ast_node_t* parent, ast_expr_t* func, const std::vector<ast_expr_t*>& args)
	{
		auto* node = new ast_expr_func_ref_t(parent);
		node->func = func;
		node->args = args;
		this->nodes.push_back(node);
		return node;
	}
	
	ast_expr_array_def_t* ast_module_t::create_expr_array_def(ast_node_t* parent, const std::vector<ast_expr_t*>& items)
	{
		auto* node = new ast_expr_array_def_t(parent);
		node->items = items;
		this->nodes.push_back(node);
		return node;
	}
	
	ast_expr_index_ref_t* ast_module_t::create_expr_index_ref(ast_node_t* parent, ast_expr_t* obj, ast_expr_t* key)
	{
		auto* node = new ast_expr_index_ref_t(parent);
		node->obj = obj;
		node->key = key;
		this->nodes.push_back(node);
		return node;
	}
	
	ast_expr_object_def_t* ast_module_t::create_expr_object_def(ast_node_t* parent, ast_type_t* type, const std::map<String, ast_expr_t*>& members)
	{
		auto* node = new ast_expr_object_def_t(parent);
		node->type = type;
		node->members = members;
		this->nodes.push_back(node);
		return node;
	}
	
	ast_expr_object_ref_t* ast_module_t::create_expr_object_ref(ast_node_t* parent, ast_expr_t* obj, const String& key)
	{
		auto* node = new ast_expr_object_ref_t(parent);
		node->obj = obj;
		node->key = key;
		this->nodes.push_back(node);
		return node;
	}
	
	ast_expr_module_ref_t* ast_module_t::create_expr_module_ref(ast_node_t* parent, ast_expr_t* name)
	{
		auto* node = new ast_expr_module_ref_t(parent);
		node->name = name;
		this->nodes.push_back(node);
		return node;
	}
}