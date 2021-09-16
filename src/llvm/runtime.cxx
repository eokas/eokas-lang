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

    llvm::Type* type_ret = llvm::Type::getInt32Ty(context);

    std::vector<llvm::Type*> type_args = 
    {
        llvm::Type::getInt8PtrTy(context)
    };

    auto funcType = llvm::FunctionType::get(type_ret, type_args, false);
    auto funcValue = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, name, module);
    funcValue->setCallingConv(llvm::CallingConv::C);

    return funcValue;
}

llvm::Function* llvm_define_function_itoa(llvm::LLVMContext& context, llvm::Module* module)
{
    llvm::StringRef name = "itoa";

    llvm::Type* type_ret = llvm::Type::getInt8PtrTy(context);

    std::vector<llvm::Type*> type_args = 
    {
        llvm::Type::getInt32Ty(context),
        llvm::Type::getInt8PtrTy(context),
        llvm::Type::getInt32Ty(context)
    };

    auto funcType = llvm::FunctionType::get(type_ret, type_args, false);
    auto funcValue = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, name, module);
    funcValue->setCallingConv(llvm::CallingConv::C);

    return funcValue;
}

_EndNamespace(eokas)
