
#ifndef _EOKAS_LLVM_RUNTIME_H_
#define _EOKAS_LLVM_RUNTIME_H_

#include <libarchaism/archaism.h>

namespace llvm {
    class LLVMContext;

    class Type;

    class Value;

    class Function;

    class Module;

    class BasicBlock;
}

_BeginNamespace(eokas)

llvm::Function *llvm_define_cfunc_puts(llvm::LLVMContext &context, llvm::Module *module);

llvm::Function *llvm_define_cfunc_printf(llvm::LLVMContext &context, llvm::Module *module);

llvm::Function *llvm_define_cfunc_sprintf(llvm::LLVMContext &context, llvm::Module *module);

llvm::Function *llvm_define_cfunc_malloc(llvm::LLVMContext &context, llvm::Module *module);

llvm::Function *llvm_define_cfunc_free(llvm::LLVMContext &context, llvm::Module *module);

llvm::Value *llvm_invoke_cfunc_printf(llvm::BasicBlock *block, std::vector<llvm::Value *> args);

llvm::Value *llvm_invoke_code_as_string(llvm::BasicBlock *block, std::vector<llvm::Value *> args);

llvm::Value *llvm_invoke_code_print(llvm::BasicBlock *block, std::vector<llvm::Value *> args);

llvm::Type *llvm_define_type_string(llvm::LLVMContext &context, llvm::Module *module);

bool llvm_is_string(llvm::Type *type);

bool llvm_is_string(llvm::Value *value);

_EndNamespace(eokas)

#endif//_EOKAS_LLVM_RUNTIME_H_
