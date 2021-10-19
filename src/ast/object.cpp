
#include "object.h"

#include <utility>

namespace eokas
{
	ast_expr_t::ast_expr_t(ast_type_t* type)
		: type(type)
	{}
	
	ast_expr_t::~ast_expr_t()
	{
		this->type = nullptr;
	}
	
	ast_type_t* ast_expr_t::getType()
	{
		return this->type;
	}
	
	ast_symbol_t::ast_symbol_t(const String&  name, ast_type_t* type, ast_expr_t* value)
		: ast_expr_t(type)
		, name(name)
		, value(value)
	{}
	
	ast_symbol_t::~ast_symbol_t()
	{
		this->name.clear();
		this->value = nullptr;
	}
	
	const String& ast_symbol_t::getName() const
	{
		return this->name;
	}
	
	const ast_type_t* ast_symbol_t::getType() const
	{
		return this->type;
	}
	
	const ast_expr_t* ast_symbol_t::getValue() const
	{
		return this->value;
	}
	
	ast_scope_t::ast_scope_t(ast_scope_t* parent)
		: parent(parent), children(), types(), symbols()
	{ }
	
	ast_scope_t::~ast_scope_t()
	{
		this->parent = nullptr;
		_DeleteList(this->children);
		this->types.clear();
		this->symbols.clear();
	}
	
	ast_scope_t* ast_scope_t::add_child()
	{
		auto* child = new ast_scope_t(this);
		this->children.push_back(child);
		return child;
	}
	
	ast_type_t* ast_scope_t::get_type(const String& name, bool lookup)
	{
		if(!lookup)
		{
			auto iter = this->types.find(name);
			if(iter != this->types.end())
				return iter->second;
			return nullptr;
		}
		
		for (auto scope = this; scope != nullptr; scope = scope->parent)
		{
			auto iter = scope->types.find(name);
			if(iter != scope->types.end())
				return iter->second;
		}
		
		return nullptr;
	}
	
	void ast_scope_t::set_type(const String& name, ast_type_t* type)
	{
		this->types.insert(std::make_pair(name, type));
	}
	
	ast_symbol_t* ast_scope_t::get_symbol(const String& name, bool lookup)
	{
		if(!lookup)
		{
			auto iter = this->symbols.find(name);
			if(iter != this->symbols.end())
				return iter->second;
			return nullptr;
		}
		
		for (auto scope = this; scope != nullptr; scope = scope->parent)
		{
			auto iter = scope->symbols.find(name);
			if(iter != scope->symbols.end())
				return iter->second;
		}
		
		return nullptr;
	}
	
	void ast_scope_t::set_symbol(const String& name, ast_symbol_t* symbol)
	{
		this->symbols.insert(std::make_pair(name, symbol));
	}
}
