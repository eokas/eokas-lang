#include "object.h"

#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/Function.h>
#include <llvm/Support/Casting.h>

_BeginNamespace(eokas)
	
	llvm_expr_t::llvm_expr_t(llvm::Value* value, llvm::Type* type)
	{
		this->value = value;
		if(type != nullptr)
		{
			this->type = type;
		}
		else if(llvm::isa<llvm::Function>(value))
		{
			this->type = llvm::cast<llvm::Function>(value)->getFunctionType();
		}
		else
		{
			this->type = value->getType();
		}
	}
	
	llvm_expr_t::~llvm_expr_t()
	{
		this->value = nullptr;
		this->type = nullptr;
	}
	
	llvm_scope_t::llvm_scope_t(llvm_scope_t* parent)
		: parent(parent)
		, children()
		, symbols()
		, types()
	{ }
	
	llvm_scope_t::~llvm_scope_t()
	{
		this->parent = nullptr;
		_DeleteList(this->children);
		this->types.clear();
		this->symbols.clear();
	}
	
	llvm_scope_t* llvm_scope_t::addChild()
	{
		auto* child = new llvm_scope_t(this);
		this->children.push_back(child);
		return child;
	}
	
	llvm_expr_t* llvm_scope_t::getSymbol(const String& name, bool lookup)
	{
		if(lookup)
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
	
	llvm_type_t* llvm_scope_t::getType(const String& name, bool lookup)
	{
		if(lookup)
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
