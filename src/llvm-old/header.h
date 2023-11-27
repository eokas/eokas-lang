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

namespace eokas
{
	struct llvm_module_t;
	struct llvm_type_t;
	struct llvm_value_t;
	struct llvm_function_t;
	struct llvm_scope_t;
}

#endif//_EOKAS_LLVM_HEADER_H_
