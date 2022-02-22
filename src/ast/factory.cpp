
#include "factory.h"
#include "type.h"
#include "expr.h"
#include "stmt.h"

namespace eokas
{
	ast_factory_t::ast_factory_t()
		: nodes()
	{}
	
	ast_factory_t::~ast_factory_t()
	{
		_DeleteList(nodes);
	}
	
	ast_type_ref_t* ast_factory_t::create_type_ref(ast_node_t* parent)
	{
		auto* node = new ast_type_ref_t(parent);
		this->nodes.push_back(node);
		return node;
	}
	
	ast_type_array_t* ast_factory_t::create_type_array(ast_node_t* parent)
	{
		auto* node = new ast_type_array_t(parent);
		this->nodes.push_back(node);
		return node;
	}
	
	ast_type_generic_t* ast_factory_t::create_type_generic(ast_node_t* parent)
	{
		auto* node = new ast_type_generic_t(parent);
		this->nodes.push_back(node);
		return node;
	}
	
	ast_expr_trinary_t* ast_factory_t::create_expr_trinary(ast_node_t* parent)
	{
		auto* node = new ast_expr_trinary_t(parent);
		this->nodes.push_back(node);
		return node;
	}
	
	ast_expr_binary_type_t* ast_factory_t::create_expr_binary_type(ast_node_t* parent)
	{
		auto* node = new ast_expr_binary_type_t(parent);
		this->nodes.push_back(node);
		return node;
	}
	
	ast_expr_binary_value_t* ast_factory_t::create_expr_binary_value(ast_node_t* parent)
	{
		auto* node = new ast_expr_binary_value_t(parent);
		this->nodes.push_back(node);
		return node;
	}
	
	ast_expr_unary_t* ast_factory_t::create_expr_unary(ast_node_t* parent)
	{
		auto* node = new ast_expr_unary_t(parent);
		this->nodes.push_back(node);
		return node;
	}
	
	ast_expr_int_t* ast_factory_t::create_expr_int(ast_node_t* parent)
	{
		auto* node = new ast_expr_int_t(parent);
		this->nodes.push_back(node);
		return node;
	}
	
	ast_expr_float_t* ast_factory_t::create_expr_float(ast_node_t* parent)
	{
		auto* node = new ast_expr_float_t(parent);
		this->nodes.push_back(node);
		return node;
	}
	
	ast_expr_bool_t* ast_factory_t::create_expr_bool(ast_node_t* parent)
	{
		auto* node = new ast_expr_bool_t(parent);
		this->nodes.push_back(node);
		return node;
	}
	
	ast_expr_string_t* ast_factory_t::create_expr_string(ast_node_t* parent)
	{
		auto* node = new ast_expr_string_t(parent);
		this->nodes.push_back(node);
		return node;
	}
	
	ast_expr_symbol_ref_t* ast_factory_t::create_expr_symbol_ref(ast_node_t* parent)
	{
		auto* node = new ast_expr_symbol_ref_t(parent);
		this->nodes.push_back(node);
		return node;
	}
	
	ast_expr_func_def_t* ast_factory_t::create_expr_func_def(ast_node_t* parent)
	{
		auto* node = new ast_expr_func_def_t(parent);
		this->nodes.push_back(node);
		return node;
	}
	
	ast_expr_func_ref_t* ast_factory_t::create_expr_func_ref(ast_node_t* parent)
	{
		auto* node = new ast_expr_func_ref_t(parent);
		this->nodes.push_back(node);
		return node;
	}
	
	ast_expr_array_def_t* ast_factory_t::create_expr_array_def(ast_node_t* parent)
	{
		auto* node = new ast_expr_array_def_t(parent);
		this->nodes.push_back(node);
		return node;
	}
	
	ast_expr_index_ref_t* ast_factory_t::create_expr_index_ref(ast_node_t* parent)
	{
		auto* node = new ast_expr_index_ref_t(parent);
		this->nodes.push_back(node);
		return node;
	}
	
	ast_expr_object_def_t* ast_factory_t::create_expr_object_def(ast_node_t* parent)
	{
		auto* node = new ast_expr_object_def_t(parent);
		this->nodes.push_back(node);
		return node;
	}
	
	ast_expr_object_ref_t* ast_factory_t::create_expr_object_ref(ast_node_t* parent)
	{
		auto* node = new ast_expr_object_ref_t(parent);
		this->nodes.push_back(node);
		return node;
	}
	
	ast_expr_module_ref_t* ast_factory_t::create_expr_module_ref(ast_node_t* parent)
	{
		auto* node = new ast_expr_module_ref_t(parent);
		this->nodes.push_back(node);
		return node;
	}
	
	ast_stmt_struct_member_t* ast_factory_t::create_stmt_struct_member(ast_node_t* parent)
	{
		auto* node = new ast_stmt_struct_member_t(parent);
		this->nodes.push_back(node);
		return node;
	}
	
	ast_stmt_struct_def_t* ast_factory_t::create_stmt_struct_def(ast_node_t* parent)
	{
		auto* node = new ast_stmt_struct_def_t(parent);
		this->nodes.push_back(node);
		return node;
	}
	
	ast_stmt_enum_def_t* ast_factory_t::create_stmt_enum_def(ast_node_t* parent)
	{
		auto* node = new ast_stmt_enum_def_t(parent);
		this->nodes.push_back(node);
		return node;
	}
	
	ast_stmt_proc_def_t* ast_factory_t::create_stmt_proc_def(ast_node_t* parent)
	{
		auto* node = new ast_stmt_proc_def_t(parent);
		this->nodes.push_back(node);
		return node;
	}
	
	ast_stmt_symbol_def_t* ast_factory_t::create_stmt_symbol_def(ast_node_t* parent)
	{
		auto* node = new ast_stmt_symbol_def_t(parent);
		this->nodes.push_back(node);
		return node;
	}
	
	ast_stmt_break_t* ast_factory_t::create_stmt_break(ast_node_t* parent)
	{
		auto* node = new ast_stmt_break_t(parent);
		this->nodes.push_back(node);
		return node;
	}
	
	ast_stmt_continue_t* ast_factory_t::create_stmt_continue(ast_node_t* parent)
	{
		auto* node = new ast_stmt_continue_t(parent);
		this->nodes.push_back(node);
		return node;
	}
	
	ast_stmt_return_t* ast_factory_t::create_stmt_return(ast_node_t* parent)
	{
		auto* node = new ast_stmt_return_t(parent);
		this->nodes.push_back(node);
		return node;
	}
	
	ast_stmt_if_t* ast_factory_t::create_stmt_if(ast_node_t* parent)
	{
		auto* node = new ast_stmt_if_t(parent);
		this->nodes.push_back(node);
		return node;
	}
	
	ast_stmt_loop_t* ast_factory_t::create_stmt_loop(ast_node_t* parent)
	{
		auto* node = new ast_stmt_loop_t(parent);
		this->nodes.push_back(node);
		return node;
	}
	
	ast_stmt_block_t* ast_factory_t::create_stmt_block(ast_node_t* parent)
	{
		auto* node = new ast_stmt_block_t(parent);
		this->nodes.push_back(node);
		return node;
	}
	
	ast_stmt_call_t* ast_factory_t::create_stmt_call(ast_node_t* parent)
	{
		auto* node = new ast_stmt_call_t(parent);
		this->nodes.push_back(node);
		return node;
	}
	
	ast_stmt_assign_t* ast_factory_t::create_stmt_assign(ast_node_t* parent)
	{
		auto* node = new ast_stmt_assign_t(parent);
		this->nodes.push_back(node);
		return node;
	}
}
