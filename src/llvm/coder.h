#ifndef _EOKAS_LLVM_CODER_H_
#define _EOKAS_LLVM_CODER_H_

#include "header.h"

_BeginNamespace(eokas)
	
	llvm::Module* llvm_encode(llvm::LLVMContext& context, ast_module_t* module);
	
	llvm::Module* llvm_encode_test(llvm::LLVMContext& context, ast_module_t* module);

_EndNamespace(eokas)

#endif//_EOKAS_LLVM_CODER_H_
