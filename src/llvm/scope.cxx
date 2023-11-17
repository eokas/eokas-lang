#include "scope.h"

namespace eokas
{
	llvm_scope_t::llvm_scope_t(llvm_scope_t* parent, llvm_function_t* func)
		: parent(parent), func(func), children(), types(), values()
	{ }
	
	llvm_scope_t::~llvm_scope_t()
	{
		this->parent = nullptr;
		this->func = nullptr;
		_DeleteList(this->children);
	}
	
	llvm_scope_t* llvm_scope_t::add_child(llvm_function_t* f)
	{
		auto* child = new llvm_scope_t(this, f != nullptr ? f : this->func);
		this->children.push_back(child);
		return child;
	}
	
	bool llvm_scope_t::add_type(const String& name, llvm_type_t* type)
	{
		bool ret = this->types.add(name, type);
		return ret;
	}
	
	llvm_type_t* llvm_scope_t::get_type(const String& name, bool lookup)
	{
		if(lookup)
		{
			for (auto scope = this; scope != nullptr; scope = scope->parent)
			{
				auto* schema = scope->types.get(name);
				if(schema != nullptr)
					return schema;
			}
			return nullptr;
		}
		else
		{
			return this->types.get(name);
		}
	}
	
	bool llvm_scope_t::add_value(const String& name, llvm_value_t* value)
	{
		bool ret = this->values.add(name, value);
		return ret;
	}
	
	llvm_value_t* llvm_scope_t::get_value(const String& name, bool lookup)
	{
		if(lookup)
		{
			for (auto scope = this; scope != nullptr; scope = scope->parent)
			{
				auto symbol = scope->values.get(name);
				if(symbol != nullptr)
					return symbol;
			}
			return nullptr;
		}
		else
		{
			return this->values.get(name);
		}
	}
}
