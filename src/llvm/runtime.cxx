#include "runtime.h"

#include <llvm/ADT/APFloat.h>
#include <llvm/ADT/STLExtras.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Verifier.h>

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <map>
#include <memory>
#include <string>
#include <vector>

_BeginNamespace(eokas)

llvm::Function *llvm_define_cfunc_puts(llvm::LLVMContext &context, llvm::Module *module) {
    llvm::StringRef name = "puts";

    llvm::Type *ret = llvm::Type::getInt32Ty(context);

    std::vector < llvm::Type * > args = {
        llvm::Type::getInt8PtrTy(context)
    };

    llvm::AttributeList attrs;

    auto funcType = llvm::FunctionType::get(ret, args, false);
    auto funcValue = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, name, module);
    funcValue->setCallingConv(llvm::CallingConv::C);
    funcValue->setAttributes(attrs);

    return funcValue;
}

llvm::Function *llvm_define_cfunc_printf(llvm::LLVMContext &context, llvm::Module *module) {
    llvm::StringRef name = "printf";

    llvm::Type *ret = llvm::Type::getInt32Ty(context);

    std::vector < llvm::Type * > args = {
        llvm::Type::getInt8PtrTy(context)
    };

    llvm::AttributeList attrs;

    auto funcType = llvm::FunctionType::get(ret, args, true);
    auto funcValue = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, name, module);
    funcValue->setCallingConv(llvm::CallingConv::C);
    funcValue->setAttributes(attrs);

    return funcValue;
}

llvm::Function *llvm_define_cfunc_sprintf(llvm::LLVMContext &context, llvm::Module *module) {
    llvm::StringRef name = "sprintf";

    llvm::Type *ret = llvm::Type::getInt32Ty(context);

    std::vector < llvm::Type * > args = {
        llvm::Type::getInt8PtrTy(context),
        llvm::Type::getInt8PtrTy(context)
    };

    llvm::AttributeList attrs;

    auto funcType = llvm::FunctionType::get(ret, args, true);
    auto funcValue = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, name, module);
    funcValue->setCallingConv(llvm::CallingConv::C);
    funcValue->setAttributes(attrs);

    return funcValue;
}

llvm::Function *llvm_define_cfunc_malloc(llvm::LLVMContext &context, llvm::Module *module) {
    llvm::StringRef name = "malloc";

    llvm::Type *ret = llvm::Type::getInt8PtrTy(context);

    std::vector < llvm::Type * > args = {
        llvm::Type::getInt32Ty(context)
    };

    llvm::AttributeList attrs;

    auto funcType = llvm::FunctionType::get(ret, args, false);
    auto funcValue = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, name, module);
    funcValue->setCallingConv(llvm::CallingConv::C);
    funcValue->setAttributes(attrs);

    return funcValue;
}

llvm::Function *llvm_define_cfunc_free(llvm::LLVMContext &context, llvm::Module *module) {
    llvm::StringRef name = "free";

    llvm::Type *ret = llvm::Type::getVoidTy(context);

    std::vector < llvm::Type * > args = {
        llvm::Type::getInt8PtrTy(context)
    };

    llvm::AttributeList attrs;

    auto funcType = llvm::FunctionType::get(ret, args, false);
    auto funcValue = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, name, module);
    funcValue->setCallingConv(llvm::CallingConv::C);
    funcValue->setAttributes(attrs);

    return funcValue;
}

llvm::Value *llvm_invoke_cfunc_printf(llvm::BasicBlock *block, std::vector<llvm::Value *> args) {
    llvm::LLVMContext &context = block->getContext();
    llvm::Module *module = block->getModule();

    llvm::IRBuilder<> builder(context);
    builder.SetInsertPoint(block);

    auto sprintf = module->getFunction("printf");
    llvm::Value *ret = builder.CreateCall(sprintf, args);

    return ret;
}

llvm::Value *llvm_invoke_code_as_string(llvm::BasicBlock *block, std::vector<llvm::Value *> args) {
    llvm::LLVMContext &context = block->getContext();
    llvm::Module *module = block->getModule();

    llvm::IRBuilder<> builder(context);
    builder.SetInsertPoint(block);

    llvm::Value *val = args[0];
    llvm::Type *vt = val->getType();

    // val is string
    if (vt->isPointerTy()) {
        auto vit = vt->getPointerElementType();
        if(vit->isIntegerTy(8)) {
            return args[0];
        }
    }

    llvm::Value *str = builder.CreateAlloca(
        llvm::ArrayType::get(
            llvm::Type::getInt8Ty(context),
            64
        )
    );

    llvm::StringRef vf = "%x";
    if (vt->isIntegerTy()) vf = "%d";
    else if (vt->isFloatingPointTy()) vf = "%f";

    llvm::Value *fmt = builder.CreateGlobalString(vf);

    auto sprintf = module->getFunction("sprintf");
    builder.CreateCall(sprintf, {str, fmt, val});

    return str;
}

llvm::Value *llvm_invoke_code_print(llvm::BasicBlock *block, std::vector<llvm::Value *> args) {
    llvm::LLVMContext &context = block->getContext();
    llvm::Module *module = block->getModule();

    llvm::IRBuilder<> builder(context);
    builder.SetInsertPoint(block);

    llvm::Value *str = llvm_invoke_code_as_string(block, args);
    builder.SetInsertPoint(block);

    auto puts = module->getFunction("puts");
    llvm::Value *ret = builder.CreateCall(puts, {str});

    return ret;
}

_EndNamespace(eokas)