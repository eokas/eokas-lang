
#include "module.h"
#include "factory.h"
#include "object.h"
#include "type.h"
#include "expr.h"
#include "stmt.h"

namespace eokas
{
	ast_module_t::ast_module_t()
		: factory(new ast_factory_t())
		, entry(nullptr)
		, func(nullptr)
		, scope(nullptr)
	{
		this->entry = factory->create_expr_func_def(nullptr);
		this->func = this->entry;
		this->scope = new ast_scope_t(nullptr);
	}
	
	ast_module_t::~ast_module_t()
	{
		_DeletePointer(this->factory);
		this->entry = nullptr;
		this->func = nullptr;
		_DeletePointer(this->scope);
	}
	
	ast_factory_t* ast_module_t::get_factory() const
	{
		return this->factory;
	}
	
	ast_expr_func_def_t* ast_module_t::get_entry() const
	{
		return this->entry;
	}
	
	ast_expr_func_def_t* ast_module_t::get_func() const
	{
		return this->func;
	}
	
	ast_scope_t* ast_module_t::get_scope() const
	{
		return this->scope;
	}
}