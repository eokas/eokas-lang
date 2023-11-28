#ifndef _EOKAS_LLVM_CODER_H_
#define _EOKAS_LLVM_CODER_H_

#include "header.h"
#include "models.h"

namespace eokas
{
	llvm_module_t* llvm_encode(llvm::LLVMContext& context, ast_node_module_t* module);
}

#endif//_EOKAS_LLVM_CODER_H_
