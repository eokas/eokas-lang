#ifndef _EOKAS_LLVM_ENGINE_H_
#define _EOKAS_LLVM_ENGINE_H_

#include "header.h"

_BeginNamespace(eokas)
	
	bool llvm_jit(ast_module_t* module);
	bool llvm_aot(ast_module_t* module);
	
_EndNamespace(eokas)

#endif//_EOKAS_LLVM_ENGINE_H_
