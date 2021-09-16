
#ifndef _EOKAS_LLVM_RUNTIME_H_
#define _EOKAS_LLVM_RUNTIME_H_

#include <libarchaism/archaism.h>

namespace llvm
{
    class LLVMContext;
    class Type;
    class Value;
    class Function;
    class Module;
}

_BeginNamespace(eokas)

llvm::Function* llvm_define_function_puts(llvm::LLVMContext& context, llvm::Module* module);
llvm::Function* llvm_define_function_itoa(llvm::LLVMContext& context, llvm::Module* module);

_EndNamespace(eokas)

#endif//_EOKAS_LLVM_RUNTIME_H_
