#ifndef _EOKAS_LLVM_HEADER_H_
#define _EOKAS_LLVM_HEADER_H_

#include "../ast/ast.h"

namespace llvm
{
	class LLVMContext;
	class Type;
	class Value;
	class Function;
	class Module;
	class BasicBlock;
}

_BeginNamespace(eokas)
	
	struct llvm_module_t;
_EndNamespace(eokas)

#endif//_EOKAS_LLVM_HEADER_H_
