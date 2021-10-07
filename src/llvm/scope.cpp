
#include "scope.h"
#include "expr.h"

#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>

_BeginNamespace(eokas)
	
	llvm_scope_t::llvm_scope_t(llvm_scope_t *parent)
		: parent(parent), children(), symbols(), types()
	{ }
	
	llvm_scope_t::~llvm_scope_t()
	{
		this->parent = nullptr;
		_DeleteList(this->children);
		this->types.clear();
		this->symbols.clear();
	}
	
	llvm_scope_t *llvm_scope_t::addChild()
	{
		llvm_scope_t *child = new llvm_scope_t(this);
		this->children.push_back(child);
		return child;
	}
	
	llvm_expr_t *llvm_scope_t::getSymbol(const String &name, bool lookUp)
	{
		if(lookUp)
		{
			for (auto scope = this; scope != nullptr; scope = scope->parent)
			{
				auto iter = scope->symbols.find(name);
				if(iter != scope->symbols.end())
					return iter->second;
			}
			return nullptr;
		}
		else
		{
			auto iter = this->symbols.find(name);
			if(iter != this->symbols.end())
				return iter->second;
			return nullptr;
		}
	}
	
	llvm::Type *llvm_scope_t::getType(const String &name, bool lookUp)
	{
		if(lookUp)
		{
			for (auto scope = this; scope != nullptr; scope = scope->parent)
			{
				auto iter = scope->types.find(name);
				if(iter != scope->types.end())
					return iter->second;
			}
			return nullptr;
		}
		else
		{
			auto iter = this->types.find(name);
			if(iter != this->types.end())
				return iter->second;
			return nullptr;
		}
	}

_EndNamespace(eokas)
