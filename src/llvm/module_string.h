
#ifndef _EOKAS_LLVM_MODULE_STRING_H_
#define _EOKAS_LLVM_MODULE_STRING_H_

#include "header.h"
#include "scope.h"

_BeginNamespace(eokas)
	
	class llvm_module_string_t
	{
		explicit llvm_module_string_t(llvm::LLVMContext &context, llvm::Module *module, llvm_scope_t *scope);
		
		virtual ~llvm_module_string_t();
	};

_EndNamespace(eokas)

#endif//_EOKAS_LLVM_MODULE_STRING_H_
