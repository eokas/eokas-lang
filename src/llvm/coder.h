#ifndef _EOKAS_LLVM_CODER_H_
#define _EOKAS_LLVM_CODER_H_

#include "../ast/ast.h"

namespace llvm {
    class Module;
}

_BeginNamespace(eokas)

std::unique_ptr<llvm::Module> llvm_encode(ast_module_t* module);

_EndNamespace(eokas)

#endif//_EOKAS_LLVM_CODER_H_
