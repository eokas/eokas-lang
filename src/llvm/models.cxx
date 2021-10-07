#include "models.h"

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

	llvm_model_t::llvm_model_t(llvm::LLVMContext& context)
		: context(context)
	{
		type_void = llvm::Type::getVoidTy(context);
		type_i8 = llvm::Type::getInt8Ty(context);
		type_i16 = llvm::Type::getInt16Ty(context);
		type_i32 = llvm::Type::getInt32Ty(context);
		type_i64 = llvm::Type::getInt64Ty(context);
		type_u8 = llvm::Type::getInt8Ty(context);
		type_u16 = llvm::Type::getInt16Ty(context);
		type_u32 = llvm::Type::getInt32Ty(context);
		type_u64 = llvm::Type::getInt64Ty(context);
		type_f32 = llvm::Type::getFloatTy(context);
		type_f64 = llvm::Type::getDoubleTy(context);
		type_bool = llvm::Type::getInt1Ty(context);
		type_string = this->define_type_string();
		type_i8_ptr = llvm::Type::getInt8PtrTy(context);
		type_string_ptr = llvm::PointerType::get(type_string, 0);
		
		const_zero = llvm::ConstantInt::get(context, llvm::APInt(32, 0));
	}
	
	llvm::Type* llvm_model_t::define_type_string()
	{
		llvm::StructType* stringType = llvm::StructType::create(context, "struct.string");
		
		std::vector<llvm::Type*> body;
		body.push_back(llvm::Type::getInt8PtrTy(context));
		stringType->setBody(body);
		
		return stringType;
	}
	
	llvm::Function* llvm_model_t::declare_cfunc_puts(llvm::Module* module)
	{
		llvm::StringRef name = "puts";
		llvm::Type* ret = type_i32;
		std::vector<llvm::Type*> args = {type_i8_ptr };
		bool varg = false;
		
		llvm::AttributeList attrs;
		
		auto funcType = llvm::FunctionType::get(ret, args, varg);
		auto funcValue = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, name, module);
		funcValue->setCallingConv(llvm::CallingConv::C);
		funcValue->setAttributes(attrs);
		
		return funcValue;
	}
	
	llvm::Function* llvm_model_t::declare_cfunc_printf(llvm::Module* module)
	{
		llvm::StringRef name = "printf";
		llvm::Type* ret = type_i32;
		std::vector<llvm::Type*> args = {type_i8_ptr };
		bool varg = true;
		
		llvm::AttributeList attrs;
		
		auto funcType = llvm::FunctionType::get(ret, args, varg);
		auto funcValue = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, name, module);
		funcValue->setCallingConv(llvm::CallingConv::C);
		funcValue->setAttributes(attrs);
		
		return funcValue;
	}
	
	llvm::Function* llvm_model_t::declare_cfunc_sprintf(llvm::Module* module)
	{
		llvm::StringRef name = "sprintf";
		llvm::Type* ret = type_i32;
		std::vector<llvm::Type*> args = {type_i8_ptr, type_i8_ptr };
		bool varg = true;
		
		llvm::AttributeList attrs;
		
		auto funcType = llvm::FunctionType::get(ret, args, varg);
		auto funcValue = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, name, module);
		funcValue->setCallingConv(llvm::CallingConv::C);
		funcValue->setAttributes(attrs);
		
		return funcValue;
	}
	
	llvm::Function* llvm_model_t::declare_cfunc_malloc(llvm::Module* module)
	{
		llvm::StringRef name = "malloc";
		llvm::Type* ret = type_i8_ptr;
		std::vector<llvm::Type*> args = { type_i32 };
		bool varg = false;
		
		llvm::AttributeList attrs;
		
		auto funcType = llvm::FunctionType::get(ret, args, varg);
		auto funcValue = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, name, module);
		funcValue->setCallingConv(llvm::CallingConv::C);
		funcValue->setAttributes(attrs);
		
		return funcValue;
	}
	
	llvm::Function* llvm_model_t::declare_cfunc_free(llvm::Module* module)
	{
		llvm::StringRef name = "free";
		llvm::Type* ret = type_void;
		std::vector<llvm::Type*> args = {type_i8_ptr };
		bool varg = false;
		
		llvm::AttributeList attrs;
		
		auto funcType = llvm::FunctionType::get(ret, args, varg);
		auto funcValue = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, name, module);
		funcValue->setCallingConv(llvm::CallingConv::C);
		funcValue->setAttributes(attrs);
		
		return funcValue;
	}
	
	llvm::Function* llvm_model_t::define_func_print(llvm::Module* module)
	{
		llvm::StringRef name = "print";
		llvm::Type* ret = type_i32;
		std::vector<llvm::Type*> args = { type_string_ptr };
		bool varg = false;
		
		llvm::AttributeList attrs;
		
		auto funcType = llvm::FunctionType::get(ret, args, varg);
		auto funcValue = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, name, module);
		funcValue->setCallingConv(llvm::CallingConv::C);
		funcValue->setAttributes(attrs);
		
		llvm::IRBuilder<> builder(context);
		
		llvm::BasicBlock* entry = llvm::BasicBlock::Create(context, "entry", funcValue);
		builder.SetInsertPoint(entry);
		
		llvm::Value* arg0 = funcValue->getArg(0);
		
		llvm::Value* i8ptr = builder.CreateStructGEP(arg0, 0);
		llvm::Value* str = builder.CreateLoad(i8ptr);
		
		llvm::Value* retval = llvm_invoke_code_print(module, builder, {str});
		
		builder.CreateRet(retval);
		
		return funcValue;
	}
	
	llvm::Value* llvm_get_value(llvm::IRBuilder<>& builder, llvm::Value* value)
	{
		llvm::Type* type = value->getType();
		while (type->isPointerTy())
		{
			if(llvm::isa<llvm::Function>(value))
				break;
			if(type->getPointerElementType()->isFunctionTy())
				break;
			value = builder.CreateLoad(value);
			type = value->getType();
		}
		return value;
	}
	
	llvm::Value* llvm_ref_value(llvm::IRBuilder<>& builder, llvm::Value* value)
	{
		llvm::Type* type = value->getType();
		while (type->isPointerTy() && type->getPointerElementType()->isPointerTy())
		{
			value = builder.CreateLoad(value);
			type = value->getType();
		}
		return value;
	}
	
	llvm::Value* llvm_invoke_code_as_string(llvm::Module* module, llvm::IRBuilder<>& builder, std::vector<llvm::Value*> args)
	{
		llvm::LLVMContext& context = module->getContext();

		llvm::Value* val = args[0];
		llvm::Type* vt = val->getType();
		
		// val is string
		if(vt->isPointerTy())
		{
			auto vit = vt->getPointerElementType();
			if(vit->isIntegerTy(8))
			{
				return args[0];
			}
		}
		
		llvm::Value* str = builder.CreateAlloca(
			llvm::ArrayType::get(
				llvm::Type::getInt8Ty(context),
				64
			)
		);
		
		llvm::StringRef vf = "%x";
		if(vt->isIntegerTy()) vf = "%d";
		else if(vt->isFloatingPointTy()) vf = "%f";
		
		llvm::Value* fmt = builder.CreateGlobalString(vf);
		
		auto sprintf = module->getFunction("sprintf");
		builder.CreateCall(sprintf, {str, fmt, val});
		
		return str;
	}
	
	llvm::Value* llvm_invoke_code_print(llvm::Module* module, llvm::IRBuilder<>& builder, std::vector<llvm::Value*> args)
	{
		llvm::LLVMContext& context = module->getContext();
		
		llvm::Value* str = llvm_invoke_code_as_string(module, builder, args);

		auto puts = module->getFunction("puts");
		llvm::Value* ret = builder.CreateCall(puts, {str});
		
		return ret;
	}
	
_EndNamespace(eokas)
