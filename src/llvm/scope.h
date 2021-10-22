#ifndef _EOKAS_LLVM_SCOPE_H_
#define _EOKAS_LLVM_SCOPE_H_

#include "header.h"

_BeginNamespace(eokas)
	
	struct llvm_expr_t;
	
	struct llvm_scope_t
	{
		llvm_scope_t* parent;
		std::vector<llvm_scope_t*> children;
		
		std::map<String, llvm_expr_t*> symbols;
		std::map<String, llvm::Type*> types;
		
		explicit llvm_scope_t(llvm_scope_t* parent);
		virtual ~llvm_scope_t();
		
		llvm_scope_t* addChild();
		llvm_expr_t* getSymbol(const String& name, bool lookUp);
		llvm::Type* getType(const String& name, bool lookUp);
	};
_EndNamespace(eokas)

#endif//_EOKAS_LLVM_SCOPE_H_
