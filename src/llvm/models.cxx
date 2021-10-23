#include "models.h"

#include <llvm/ADT/APFloat.h>
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
	
	llvm_model_t::llvm_model_t(llvm::LLVMContext& context) : context(context)
	{
		type_void = llvm::Type::getVoidTy(context);
		type_i8 = llvm::Type::getInt8Ty(context);
		type_i32 = llvm::Type::getInt32Ty(context);
		type_i64 = llvm::Type::getInt64Ty(context);
		type_u8 = llvm::Type::getInt8Ty(context);
		type_u32 = llvm::Type::getInt32Ty(context);
		type_u64 = llvm::Type::getInt64Ty(context);
		type_f32 = llvm::Type::getFloatTy(context);
		type_f64 = llvm::Type::getDoubleTy(context);
		type_bool = llvm::Type::getInt1Ty(context);
		type_string = this->define_type_string();
		type_i8_ptr = llvm::Type::getInt8PtrTy(context);
		type_string_ptr = llvm::PointerType::get(type_string, 0);
		
		default_i8 = llvm::ConstantInt::get(context, llvm::APInt(8, 0));
		default_i32 = llvm::ConstantInt::get(context, llvm::APInt(32, 0));
		default_i64 = llvm::ConstantInt::get(context, llvm::APInt(64, 0));
		default_u8 = llvm::ConstantInt::get(context, llvm::APInt(8, 0));
		default_u32 = llvm::ConstantInt::get(context, llvm::APInt(32, 0));
		default_u64 = llvm::ConstantInt::get(context, llvm::APInt(64, 0));
		default_f32 = llvm::ConstantFP::get(context, llvm::APFloat(0.0f));
		default_f64 = llvm::ConstantFP::get(context, llvm::APFloat(0.0));
		default_bool = llvm::ConstantInt::get(context, llvm::APInt(1, 0));
		default_ptr = llvm::ConstantPointerNull::get(type_void->getPointerTo());
	}
	
	llvm::Value* llvm_model_t::get_default_value_by_type(llvm::Type* type) const
	{
		if(type == type_i8)
			return default_i8;
		if(type == type_i32)
			return default_i32;
		if(type == type_i64)
			return default_i64;
		if(type == type_u8)
			return default_u8;
		if(type == type_u32)
			return default_u32;
		if(type == type_u64)
			return default_u64;
		if(type == type_f32)
			return default_f32;
		if(type == type_f64)
			return default_f64;
		if(type == type_bool)
			return default_bool;
		return default_ptr;
	}
	
	llvm::Value* llvm_model_t::get_value(llvm::IRBuilder<>& builder, llvm::Value* value)
	{
		llvm::Type* type = value->getType();
		while (type->isPointerTy())
		{
			if(llvm::isa<llvm::Function>(value))
				break;
			if(type->getPointerElementType()->isFunctionTy())
				break;
			if(type->getPointerElementType()->isStructTy())
				break;
			value = builder.CreateLoad(value);
			type = value->getType();
		}
		return value;
	}
	
	llvm::Value* llvm_model_t::ref_value(llvm::IRBuilder<>& builder, llvm::Value* value)
	{
		llvm::Type* type = value->getType();
		while (type->isPointerTy() && type->getPointerElementType()->isPointerTy())
		{
			value = builder.CreateLoad(value);
			type = value->getType();
		}
		return value;
	}
	
	llvm::Type* llvm_model_t::define_type_string()
	{
		llvm::StructType* stringType = llvm::StructType::create(context, "Type.String.Struct");
		
		std::vector<llvm::Type*> body;
		body.push_back(llvm::Type::getInt8PtrTy(context));
		stringType->setBody(body);
		
		return stringType;
	}
	
	llvm::Function* llvm_model_t::declare_cfunc_puts(llvm::Module* module)
	{
		llvm::StringRef name = "puts";
		llvm::Type* ret = type_i32;
		std::vector<llvm::Type*> args = {type_i8_ptr};
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
		std::vector<llvm::Type*> args = {type_i8_ptr};
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
		std::vector<llvm::Type*> args = {type_i8_ptr, type_i8_ptr};
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
		std::vector<llvm::Type*> args = {type_i64};
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
		std::vector<llvm::Type*> args = {type_i8_ptr};
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
		std::vector<llvm::Type*> args = {type_string_ptr};
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
		llvm::Value* cstr = this->string_to_cstr(module, funcValue, builder, arg0);
		llvm::Value* retval = this->print(module, funcValue, builder, {cstr});
		
		builder.CreateRet(retval);
		
		return funcValue;
	}
	
	llvm::Value* llvm_model_t::make(llvm::Module* module, llvm::Function* func, llvm::IRBuilder<>& builder, llvm::Type* type)
	{
		auto mallocF = module->getFunction("malloc");
		llvm::Constant* len = llvm::ConstantExpr::getSizeOf(type);
		llvm::Value* ptr = builder.CreateCall(mallocF, {len});
		llvm::Value* val = builder.CreateBitCast(ptr, type->getPointerTo());
		return val;
	}
	
	void llvm_model_t::free(llvm::Module* module, llvm::Function* func, llvm::IRBuilder<>& builder, llvm::Value* ptr)
	{
		auto freeF = module->getFunction("free");
		builder.CreateCall(freeF, {ptr});
	}
	
	llvm::Value* llvm_model_t::string_to_cstr(llvm::Module* module, llvm::Function* func, llvm::IRBuilder<>& builder, llvm::Value* val)
	{
		llvm::Value* ptr = builder.CreateStructGEP(type_string, val, 0);
		llvm::Value* cstr = builder.CreateLoad(ptr);
		return cstr;
	}
	
	llvm::Value* llvm_model_t::bool_to_cstr(llvm::Module* module, llvm::Function* func, llvm::IRBuilder<>& builder, llvm::Value* val)
	{
		llvm::BasicBlock* branch_begin = llvm::BasicBlock::Create(context, "branch.begin", func);
		llvm::BasicBlock* branch_true = llvm::BasicBlock::Create(context, "branch.true", func);
		llvm::BasicBlock* branch_false = llvm::BasicBlock::Create(context, "branch.false", func);
		llvm::BasicBlock* branch_end = llvm::BasicBlock::Create(context, "branch.end", func);
		
		builder.CreateBr(branch_begin);
		
		builder.SetInsertPoint(branch_begin);
		builder.CreateCondBr(val, branch_true, branch_false);
		
		builder.SetInsertPoint(branch_true);
		auto val_true = builder.CreateGlobalString("true");
		builder.CreateBr(branch_end);
		
		builder.SetInsertPoint(branch_false);
		auto val_false = builder.CreateGlobalString("false");
		builder.CreateBr(branch_end);
		
		builder.SetInsertPoint(branch_end);
		
		llvm::PHINode* phi = builder.CreatePHI(type_i8_ptr, 2);
		phi->addIncoming(val_true, branch_true);
		phi->addIncoming(val_false, branch_false);
		
		return phi;
	}
	
	llvm::Value* llvm_model_t::number_to_cstr(llvm::Module* module, llvm::Function* func, llvm::IRBuilder<>& builder, llvm::Value* val)
	{
		llvm::Type* vt = val->getType();
		
		llvm::Value* buf = builder.CreateAlloca(llvm::ArrayType::get(type_i8, 64));
		
		llvm::StringRef vf = "%x";
		if(vt->isIntegerTy())
			vf = "%d";
		else if(vt->isFloatingPointTy())
			vf = "%f";
		
		llvm::Value* fmt = builder.CreateGlobalString(vf);
		
		auto sprintf = module->getFunction("sprintf");
		builder.CreateCall(sprintf, {buf, fmt, val});
		
		return buf;
	}
	
	llvm::Value* llvm_model_t::value_to_cstr(llvm::Module* module, llvm::Function* func, llvm::IRBuilder<>& builder, llvm::Value* val)
	{
		llvm::Type* vt = val->getType();
		
		if(vt == type_string)
			return this->string_to_cstr(module, func, builder, val);
		
		if(vt == type_i8_ptr)
			return val;
		
		if(vt->isIntegerTy(1))
			return this->bool_to_cstr(module, func, builder, val);
		
		return this->number_to_cstr(module, func, builder, val);
	}
	
	llvm::Value* llvm_model_t::cstr_to_string(llvm::Module* module, llvm::Function* func, llvm::IRBuilder<>& builder, llvm::Value* val)
	{
		llvm::Value* str = this->make(module, func, builder, type_string);
		llvm::Value* ptr = builder.CreateStructGEP(type_string, str, 0);
		builder.CreateStore(val, ptr);
		return str;
	}
	
	llvm::Value* llvm_model_t::bool_to_string(llvm::Module* module, llvm::Function* func, llvm::IRBuilder<>& builder, llvm::Value* val)
	{
		llvm::Value* cstr = this->bool_to_cstr(module, func, builder, val);
		return this->cstr_to_string(module, func, builder, cstr);
	}
	
	llvm::Value* llvm_model_t::number_to_string(llvm::Module* module, llvm::Function* func, llvm::IRBuilder<>& builder, llvm::Value* val)
	{
		llvm::Value* cstr = this->number_to_cstr(module, func, builder, val);
		return this->cstr_to_string(module, func, builder, cstr);
	}
	
	llvm::Value* llvm_model_t::value_to_string(llvm::Module* module, llvm::Function* func, llvm::IRBuilder<>& builder, llvm::Value* val)
	{
		llvm::Type* vt = val->getType();
		
		if(vt == type_string)
			return val;
		
		if(vt == type_i8_ptr)
			return this->cstr_to_string(module, func, builder, val);
		
		if(vt->isIntegerTy(1))
			return this->bool_to_string(module, func, builder, val);
		
		return this->number_to_string(module, func, builder, val);
	}
	
	llvm::Value* llvm_model_t::print(llvm::Module* module, llvm::Function* func, llvm::IRBuilder<>& builder, const std::vector<llvm::Value*>& args)
	{
		llvm::Value* val = args[0];
		llvm::Value* cstr = this->value_to_cstr(module, func, builder, val);
		auto puts = module->getFunction("puts");
		llvm::Value* ret = builder.CreateCall(puts, {cstr});
		
		return ret;
	}
_EndNamespace(eokas)
