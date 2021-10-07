
#include "expr.h"

#include <llvm/Support/Casting.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/Function.h>

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
	};
_EndNamespace(eokas)
