
#include "module.h"
#include "scope.h"
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
	
	ast_expr_symbol_t* ast_module_t::create_expr_symbol(const String& name, ast_expr_t* value)
	{
		auto* node = new ast_expr_symbol_ref_t(parent);
		node->name = name;
		
	}
}