
#ifndef _EOKAS_LLVM_EXPR_H_
#define _EOKAS_LLVM_EXPR_H_

#include "header.h"

_BeginNamespace(eokas)
	
	struct llvm_expr_t
	{
		llvm::Value *value;
		llvm::Type *type;
		
		explicit llvm_expr_t(llvm::Value *value, llvm::Type *type = nullptr);
		
		virtual ~llvm_expr_t();
	};


_EndNamespace(eokas)

#endif//_EOKAS_LLVM_EXPR_H_
