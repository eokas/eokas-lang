#include "scope.h"

#include <llvm/IR/IRBuilder.h>
#include <memory>

namespace eokas
{
	llvm_scope_t::llvm_scope_t(llvm_scope_t* parent, llvm_func_builder_t* func)
		: parent(parent), func(func), children(), symbols(), types()
	{ }
	
	llvm_scope_t::~llvm_scope_t()
	{
		this->parent = nullptr;
		this->func = nullptr;
		_DeleteList(this->children);
	}
	
	llvm_scope_t* llvm_scope_t::addChild(llvm_func_builder_t* f)
	{
		auto* child = new llvm_scope_t(this, f != nullptr ? f : this->func);
		this->children.push_back(child);
		return child;
	}
	
	bool llvm_scope_t::addSymbol(const String& name, llvm::Value* expr)
	{
		auto* symbol = new llvm_symbol_t();
		symbol->scope = this;
		symbol->type = expr->getType();
		symbol->value = expr;
		
		bool ret = this->symbols.add(name, symbol);
		return ret;
	}
	
	bool llvm_scope_t::addSymbol(const String& name, llvm::Type* type)
	{
		auto* symbol = new llvm_symbol_t();
		symbol->scope = this;
		symbol->type = type;
		symbol->value = nullptr;
		
		bool ret = this->symbols.add(name, symbol);
		return ret;
	}
	
	llvm_symbol_t* llvm_scope_t::getSymbol(const String& name, bool lookup)
	{
		if(lookup)
		{
			for (auto scope = this; scope != nullptr; scope = scope->parent)
			{
				auto symbol = scope->symbols.get(name);
				if(symbol != nullptr)
					return symbol;
			}
			return nullptr;
		}
		else
		{
			return this->symbols.get(name);
		}
	}
	
	bool llvm_scope_t::addType(const String& name, llvm_type_builder_t* type)
	{
		bool ret = this->types.add(name, type);
		return ret;
	}
	
	llvm_type_builder_t* llvm_scope_t::getType(const String& name, bool lookup)
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
}
