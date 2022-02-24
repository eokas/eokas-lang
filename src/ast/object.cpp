#include "object.h"

namespace eokas
{
	ast_scope_t::ast_scope_t(ast_scope_t* parent)
		: parent(parent), children(), types(), symbols()
	{
	}
	
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
	
	ast_stmt_symbol_def_t* ast_scope_t::get_symbol(const String& name, bool lookup)
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
	
	void ast_scope_t::set_symbol(const String& name, ast_stmt_symbol_def_t* symbol)
	{
		this->symbols.insert(std::make_pair(name, symbol));
	}
}
