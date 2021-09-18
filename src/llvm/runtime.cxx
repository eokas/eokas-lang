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

llvm::Function* llvm_define_function_puts(llvm::LLVMContext& context, llvm::Module* module)
{
    llvm::StringRef name = "puts";

    llvm::Type* ret = llvm::Type::getInt32Ty(context);

    std::vector<llvm::Type*> args =
    {
        llvm::Type::getInt8PtrTy(context)
    };

    llvm::AttributeList attrs;

    auto funcType = llvm::FunctionType::get(ret, args, false);
    auto funcValue = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, name, module);
    funcValue->setCallingConv(llvm::CallingConv::C);
    funcValue->setAttributes(attrs);

    return funcValue;
}

llvm::Function* llvm_define_function_sprintf(llvm::LLVMContext& context, llvm::Module* module)
{
    llvm::StringRef name = "sprintf";

    llvm::Type* ret = llvm::Type::getInt32Ty(context);

    std::vector<llvm::Type*> args =
    {
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

llvm::Value* llvm_int2str(llvm::BasicBlock* block, std::vector<llvm::Value*> args)
{
    llvm::LLVMContext& context = block->getContext();
    llvm::Module* module = block->getModule();

    llvm::IRBuilder<> builder(context);
    builder.SetInsertPoint(block);

    llvm::Value* buf = builder.CreateAlloca(
        llvm::ArrayType::get(
            llvm::Type::getInt8Ty(context), 
            64
        )
    );

    llvm::Value* fmt = builder.CreateGlobalStringPtr("%d");

    auto x = args[0];

    auto sprintf = module->getFunction("sprintf");
    builder.CreateCall(sprintf, {buf, fmt, x});

    return buf;
}

llvm::Value* llvm_float2str(llvm::BasicBlock* block, std::vector<llvm::Value*> args)
{
    llvm::LLVMContext& context = block->getContext();
    llvm::Module* module = block->getModule();

    llvm::IRBuilder<> builder(context);
    builder.SetInsertPoint(block);

    llvm::Value* buf = builder.CreateAlloca(
        llvm::ArrayType::get(
            llvm::Type::getInt8Ty(context), 
            64
        )
    );

    llvm::Value* fmt = builder.CreateGlobalStringPtr("%f");

    auto x = args[0];

    auto sprintf = module->getFunction("sprintf");
    builder.CreateCall(sprintf, {buf, fmt, x});

    return buf;
}

llvm::Value* llvm_as_string(llvm::BasicBlock* block, std::vector<llvm::Value*> args)
{
    llvm::LLVMContext& context = block->getContext();
    llvm::Module* module = block->getModule();

    llvm::IRBuilder<> builder(context);
    builder.SetInsertPoint(block);

    llvm::Value* str = builder.CreateAlloca(
        llvm::ArrayType::get(
            llvm::Type::getInt8Ty(context), 
            64
        )
    );

    llvm::Value* val = args[0];
    llvm::Type* vt = val->getType();

    llvm::StringRef vf = "%x";
    if(vt->isIntegerTy()) vf = "%d";
    else if(vt->isFloatingPointTy()) vf = "%f";

    llvm::Value* fmt = builder.CreateGlobalStringPtr(vf);

    auto sprintf = module->getFunction("sprintf");
    builder.CreateCall(sprintf, {str, fmt, val});

    return str;
}

_EndNamespace(eokas)
