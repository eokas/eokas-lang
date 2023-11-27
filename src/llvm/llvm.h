#ifndef _EOKAS_LLVM_ENGINE_H_
#define _EOKAS_LLVM_ENGINE_H_

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
	bool llvm_jit(ast_node_module_t* module);
	bool llvm_aot(ast_node_module_t* module);
}

#endif//_EOKAS_LLVM_ENGINE_H_
