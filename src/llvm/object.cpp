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
	
	bool llvm_expr_t::is_symbol() const
	{
		if(this->type == nullptr || this->value == nullptr)
			return false;
		auto vt = this->value->getType();
		return vt->isPointerTy()
			&& vt->getPointerElementType() == this->type;
	}
	
	llvm_scope_t::llvm_scope_t(llvm_scope_t* parent, llvm::Function* func)
		: parent(parent)
        , func(func)
        , children()
		, symbols()
		, types()
	{ }
	
	llvm_scope_t::~llvm_scope_t()
	{
		this->parent = nullptr;
        this->func = nullptr;
		_DeleteList(this->children);
	}
	
	llvm_scope_t* llvm_scope_t::addChild(llvm::Function* f)
	{
		auto* child = new llvm_scope_t(this, f != nullptr ? f : this->func);
		this->children.push_back(child);
		return child;
	}

    bool llvm_scope_t::addSymbol(const String& name, llvm_expr_t* expr)
    {
        bool ret = this->symbols.add(name, expr);
        if(ret)
        {
            expr->scope = this;
        }
        return ret;
    }
	
	llvm_expr_t* llvm_scope_t::getSymbol(const String& name, bool lookup)
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

    bool llvm_scope_t::addType(const String& name, llvm_type_t* type)
    {
        bool ret = this->types.add(name, type);
        if(ret)
        {
            type->scope = this;
        }
        return ret;
    }
	
	llvm_type_t* llvm_scope_t::getType(const String& name, bool lookup)
	{
		if(lookup)
		{
			for (auto scope = this; scope != nullptr; scope = scope->parent)
			{
                auto type = scope->types.get(name);
                if(type != nullptr)
                    return type;
			}
			return nullptr;
		}
		else
		{
			return this->types.get(name);
		}
	}
	
_EndNamespace(eokas)
