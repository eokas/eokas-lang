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
	
	llvm_scope_t::llvm_scope_t(llvm_scope_t* parent, llvm::Function* func)
		: parent(parent), func(func), children(), symbols(), types()
	{
	}
	
	llvm_scope_t::~llvm_scope_t()
	{
		this->parent = nullptr;
		this->func = nullptr;
		_DeleteList(this->children);
	}
	
	llvm_scope_t* llvm_scope_t::addChild(llvm::Function* f)
	{
		auto* child = new llvm_scope_t(this, f != nullptr ? f : this->func);
		this->children.push_back(child);
		return child;
	}
	
	bool llvm_scope_t::addSymbol(const String& name, llvm::Value* expr)
	{
		bool ret = this->symbols.add(name, expr);
		return ret;
	}
	
	llvm_scope_t::llvm_scoped_symbol_t llvm_scope_t::getSymbol(const String& name, bool lookup)
	{
		llvm_scoped_symbol_t ret;
		
		if(lookup)
		{
			for (auto scope = this; scope != nullptr; scope = scope->parent)
			{
				auto symbol = scope->symbols.get(name);
				if(symbol != nullptr)
				{
					ret.scope = scope;
					ret.value = symbol;
					return ret;
				}
			}
			return ret;
		}
		else
		{
			ret.scope = this;
			ret.value = this->symbols.get(name);
			return ret;
		}
	}
	
	bool llvm_scope_t::addType(const String& name, llvm::Type* type)
	{
		bool ret = this->types.add(name, type);
		return ret;
	}
	
	llvm_scope_t::llvm_scoped_type_t llvm_scope_t::getType(const String& name, bool lookup)
	{
		llvm_scoped_type_t ret;
		
		if(lookup)
		{
			for (auto scope = this; scope != nullptr; scope = scope->parent)
			{
				auto type = scope->types.get(name);
				if(type != nullptr)
				{
					ret.scope = scope;
					ret.handle = type;
					return ret;
				}
			}
			return ret;
		}
		else
		{
			ret.scope = this;
			ret.handle = this->types.get(name);
			return ret;
		}
	}
	
	llvm_struct_t::llvm_struct_t(llvm::LLVMContext& context, const String& name)
		: context(context), name(name), members()
	{
		this->type = llvm::StructType::create(context, name.cstr());
	}
	
	llvm_struct_t::~llvm_struct_t() noexcept
	{
		_DeleteList(this->members);
	}
	
	llvm_struct_t::member_t* llvm_struct_t::add_member(const String& name, llvm::Type* type, llvm::Value* value)
	{
		auto* m = new member_t();
		m->name = name;
		m->type = type;
		m->value = value;
		this->members.push_back(m);
		
		return m;
	}
	
	llvm_struct_t::member_t* llvm_struct_t::add_member(const member_t* other)
	{
		return this->add_member(other->name, other->type, other->value);
	}
	
	llvm_struct_t::member_t* llvm_struct_t::get_member(const String& name) const
	{
		for (auto& m: this->members)
		{
			if(m->name == name)
				return m;
		}
		return nullptr;
	}
	
	llvm_struct_t::member_t* llvm_struct_t::get_member(size_t index) const
	{
		if(index>=this->members.size())
			return nullptr;
		return this->members.at(index);
	}
	
	size_t llvm_struct_t::get_member_index(const String& name) const
	{
		for (size_t index = 0; index<this->members.size(); index++)
		{
			if(this->members.at(index)->name == name)
				return index;
		}
		return -1;
	}
	
	void llvm_struct_t::resolve()
	{
		std::vector<llvm::Type*> body;
		for (auto& member: this->members)
		{
			body.push_back(member->type);
		}
		this->type->setBody(body);
	}
	
	llvm::Function* llvm_model_t::declare_func(llvm::Module& module, const String& name, llvm::Type* ret, const std::vector<llvm::Type*>& args, bool varg)
	{
		auto& context = module.getContext();
		
		llvm::AttributeList attrs;
		
		auto funcType = llvm::FunctionType::get(ret, args, varg);
		auto funcValue = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, name.cstr(), module);
		funcValue->setCallingConv(llvm::CallingConv::C);
		funcValue->setAttributes(attrs);
		
		return funcValue;
	}
	
	llvm::Function* llvm_model_t::define_func(llvm::Module& module, const String& name, llvm::Type* ret, const std::vector<llvm::Type*>& args, bool varg, const llvm_code_delegate_t& body)
	{
		auto& context = module.getContext();
		
		llvm::AttributeList attrs;
		
		auto funcType = llvm::FunctionType::get(ret, args, varg);
		auto funcValue = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, name.cstr(), module);
		funcValue->setCallingConv(llvm::CallingConv::C);
		funcValue->setAttributes(attrs);
		
		llvm::IRBuilder<> builder(context);
		
		body(context, module, funcValue, builder);
		
		return funcValue;
	}
	
	/**
 * For ref-types, transform multi-level pointer to one-level pointer.
 * For val-types, transform multi-level pointer to real value.
 * */
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
			if(type->getPointerElementType()->isArrayTy())
				break;
			value = builder.CreateLoad(value);
			type = value->getType();
		}
		return value;
	}
	
	/**
	 * Transform the multi-level pointer value to one-level pointer type value.
	 * Ignores literal values.
	 * */
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
	
	llvm_module_t::llvm_module_t(const String& name, llvm::LLVMContext& context)
		: context(context), module(name.cstr(), context), root(new llvm_scope_t(nullptr, nullptr))
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
		
		type_cstr = type_i8->getPointerTo();
		
		type_string = this->define_type_string();
		type_string_ptr = type_string->getPointerTo();
		
		type_void_ptr = type_void->getPointerTo();
		
		this->declare_func_malloc();
		this->declare_func_free();
		this->declare_func_printf();
		this->declare_func_sprintf();
		this->declare_func_strlen();
		
		this->root->addType("void", type_void);
		this->root->addType("i8", type_i8);
		this->root->addType("i32", type_i32);
		this->root->addType("i64", type_i64);
		this->root->addType("u8", type_u8);
		this->root->addType("u32", type_u32);
		this->root->addType("u64", type_u64);
		this->root->addType("f32", type_f32);;
		this->root->addType("f64", type_f64);;
		this->root->addType("bool", type_bool);;
		this->root->addType("string", type_string_ptr);
		
		this->root->addSymbol("print", this->define_func_print());
	}
	
	llvm_module_t::~llvm_module_t() noexcept
	{
		_DeleteList(this->structs);
		_DeletePointer(root);
	}
	
	String llvm_module_t::get_type_name(llvm::Type* type)
	{
		if(type == type_i8) return "i8";
		if(type == type_i16) return "i16";
		if(type == type_i32) return "i32";
		if(type == type_i64) return "i64";
		
		if(type == type_u8) return "u8";
		if(type == type_u16) return "u16";
		if(type == type_u32) return "u32";
		if(type == type_u64) return "u64";
		
		if(type == type_f32) return "f32";
		if(type == type_f64) return "f64";
		
		if(type == type_bool) return "bool";
		if(type == type_cstr) return "cstr";
		
		if(type->isStructTy()) return type->getStructName().data();
		if(type->isFunctionTy()) return "func";
		
		if(type->isPointerTy())
		{
			return String::format("Ref<%s>", this->get_type_name(type->getPointerElementType()).cstr());
		}
		
		return "";
	}
	
	llvm::Value* llvm_module_t::get_default_value(llvm::Type* type)
	{
		if(type->isIntegerTy())
		{
			auto bits = type->getIntegerBitWidth();
			return llvm::ConstantInt::get(context, llvm::APInt(bits, 0));
		}
		if(type->isFloatingPointTy())
		{
			return llvm::ConstantFP::get(context, llvm::APFloat(0.0f));
		}
		return llvm::ConstantPointerNull::get(llvm::Type::getVoidTy(context)->getPointerTo());
	}
	
	llvm_struct_t* llvm_module_t::new_struct(const String& name)
	{
		auto type = new llvm_struct_t(context, name);
		this->structs.push_back(type);
		return type;
	}
	
	llvm_struct_t* llvm_module_t::get_struct(const String& name)
	{
		auto iter = this->structs.begin();
		while(iter != this->structs.end())
		{
			auto type = *iter;
			if(type->name == name)
				return type;
			++ iter;
		}
		return nullptr;
	}
	
	llvm_struct_t* llvm_module_t::get_struct(llvm::Type* handle)
	{
		auto iter = this->structs.begin();
		while(iter != this->structs.end())
		{
			auto type = *iter;
			if(type->type == handle)
				return type;
			++iter;
		}
		return nullptr;
	}
	
	llvm::Function* llvm_module_t::declare_func_malloc()
	{
		String name = "malloc";
		llvm::Type* ret = type_cstr;
		std::vector<llvm::Type*> args = {type_i64};
		bool varg = false;
		
		return llvm_model_t::declare_func(module, name, ret, args, varg);
	}
	
	llvm::Function* llvm_module_t::declare_func_free()
	{
		String name = "free";
		llvm::Type* ret = type_void;
		std::vector<llvm::Type*> args = {type_cstr};
		bool varg = false;
		
		return llvm_model_t::declare_func(module, name, ret, args, varg);
	}
	
	llvm::Function* llvm_module_t::declare_func_printf()
	{
		String name = "printf";
		llvm::Type* ret = type_i32;
		std::vector<llvm::Type*> args = {type_cstr};
		bool varg = true;
		
		return llvm_model_t::declare_func(module, name, ret, args, varg);
	}
	
	llvm::Function* llvm_module_t::declare_func_sprintf()
	{
		String name = "sprintf";
		llvm::Type* ret = type_i32;
		std::vector<llvm::Type*> args = {type_cstr, type_cstr};
		bool varg = true;
		
		return llvm_model_t::declare_func(module, name, ret, args, varg);
	}
	
	llvm::Function* llvm_module_t::declare_func_strlen()
	{
		String name = "strlen";
		llvm::Type* ret = type_i32;
		std::vector<llvm::Type*> args = {type_cstr};
		bool varg = true;
		
		return llvm_model_t::declare_func(module, name, ret, args, varg);
	}
	
	llvm::Function* llvm_module_t::define_func_print()
	{
		String name = "print";
		llvm::Type* ret = type_i32;
		std::vector<llvm::Type*> args = {type_string_ptr};
		bool varg = false;
		
		return llvm_model_t::define_func(module, name, ret, args, varg, [&](llvm::LLVMContext& context, llvm::Module& module, llvm::Function* func, llvm::IRBuilder<>& builder)->void
		{
			llvm::BasicBlock* entry = llvm::BasicBlock::Create(context, "entry", func);
			builder.SetInsertPoint(entry);
			
			llvm::Value* arg0 = func->getArg(0);
			llvm::Value* cstr = this->cstr_from_string(func, builder, arg0);
			llvm::Value* retval = this->print(func, builder, {cstr});
			
			builder.CreateRet(retval);
		});
	}
	
	llvm::Type* llvm_module_t::define_type_array(llvm::Type* element_type)
	{
		String name = String::format("Array<%s>", this->get_type_name(element_type).cstr());

		auto structType = llvm::StructType::create(context);
		structType->setName(name.cstr());
		structType->setBody({
			llvm::ArrayType::get(element_type, 0)->getPointerTo(),
			this->type_u64
		});
		
		return structType;
	}
	
	bool llvm_module_t::is_array_type(llvm::Type* type)
	{
		if(!type->isPointerTy())
			return false;
		type = type->getPointerElementType();
		if(!type->isStructTy() || type->getStructNumElements() != 2)
			return false;
		
		auto dataT = type->getStructElementType(0);
		auto countT = type->getStructElementType(1);
		
		return dataT->isPointerTy()
			&& dataT->getPointerElementType()->isArrayTy()
			&& countT->isIntegerTy(64);
	}
	
	llvm::Type* llvm_module_t::define_type_string()
	{
		String name = "String";
		
		auto structType = llvm::StructType::create(context);
		structType->setName(name.cstr());
		structType->setBody({
			this->type_cstr,
			this->type_u64
		});
		
		return structType;
	}
	
	llvm::Value* llvm_module_t::make(llvm::Function* func, llvm::IRBuilder<>& builder, llvm::Type* type)
	{
		auto mallocF = module.getFunction("malloc");
		llvm::Constant* len = llvm::ConstantExpr::getSizeOf(type);
		llvm::Value* ptr = builder.CreateCall(mallocF, {len});
		llvm::Value* val = builder.CreateBitCast(ptr, type->getPointerTo());
		return val;
	}
	
	llvm::Value* llvm_module_t::make(llvm::Function* func, llvm::IRBuilder<>& builder, llvm::Type* type, llvm::Value* count)
	{
		auto mallocF = module.getFunction("malloc");
		llvm::Constant* stride = llvm::ConstantExpr::getSizeOf(type);
		llvm::Value* len = builder.CreateMul(stride, count);
		llvm::Value* ptr = builder.CreateCall(mallocF, {len});
		llvm::Value* val = builder.CreateBitCast(ptr, type->getPointerTo());
		return val;
	}
	
	llvm::Value* llvm_module_t::make(llvm::Function* func, llvm::IRBuilder<>& builder, llvm_struct_t* type)
	{
		auto ptr = this->make(func, builder, type->type);
		for(size_t i = 0; i < type->members.size(); i++)
		{
			auto mem = type->members.at(i);
			auto p = builder.CreateStructGEP(ptr, i);
			auto v = mem->value;
			builder.CreateStore(v, p);
		}
		return ptr;
	}
	
	void llvm_module_t::free(llvm::Function* func, llvm::IRBuilder<>& builder, llvm::Value* ptr)
	{
		auto freeF = module.getFunction("free");
		builder.CreateCall(freeF, {ptr});
	}
	
	llvm::Value* llvm_module_t::array_set(llvm::Function* func, llvm::IRBuilder<>& builder, llvm::Value* array, const llvm::ArrayRef<llvm::Value*>& elements)
	{
		auto elementT = elements[0]->getType();
		auto arrayT = llvm::ArrayType::get(elementT, elements.size());
		
		auto* dataP = builder.CreateStructGEP(array, 0);
		auto* dataV = this->make(func, builder, arrayT);
		for (size_t i = 0; i < elements.size(); i++)
		{
			auto elementP = builder.CreateConstGEP2_64(dataV, 0, i);
			auto elementV = elements[i];
			builder.CreateStore(elementV, elementP);
		}
		builder.CreateStore(dataV, dataP);
		
		auto countP = builder.CreateStructGEP(array, 1);
		auto countV = builder.getInt64(elements.size());
		builder.CreateStore(countV, countP);
		
		return array;
	}
	
	llvm::Value* llvm_module_t::array_get(llvm::Function* func, llvm::IRBuilder<>& builder, llvm::Value* array, llvm::Value* index)
	{
		auto dataP = builder.CreateStructGEP(array, 0);
		auto dataV = builder.CreateLoad(dataP);
		auto val = builder.CreateInBoundsGEP(dataV, {builder.getInt64(0), index});
		return val;
	}
	
	llvm::Value* llvm_module_t::cstr_len(llvm::Function* func, llvm::IRBuilder<>& builder, llvm::Value* val)
	{
		llvm::Value* cstr = this->cstr_from_value(func, builder, val);
		auto pfn = module.getFunction("strlen");
		llvm::Value* ret = builder.CreateCall(pfn, {cstr});
		
		return ret;
	}
	
	llvm::Value* llvm_module_t::cstr_from_value(llvm::Function* func, llvm::IRBuilder<>& builder, llvm::Value* val)
	{
		llvm::Type* vt = val->getType();
		
		if(vt == type_cstr)
			return val;
		
		if(vt == type_string_ptr)
			return this->cstr_from_string(func, builder, val);
		
		if(vt == type_bool)
			return this->cstr_from_bool(func, builder, val);
		
		return this->cstr_from_number(func, builder, val);
	}
	
	llvm::Value* llvm_module_t::cstr_from_string(llvm::Function* func, llvm::IRBuilder<>& builder, llvm::Value* str)
	{
		llvm::Value* ptr = builder.CreateStructGEP(type_string, str, 0);
		llvm::Value* cstr = builder.CreateLoad(ptr);
		return cstr;
	}
	
	llvm::Value* llvm_module_t::cstr_from_number(llvm::Function* func, llvm::IRBuilder<>& builder, llvm::Value* val)
	{
		llvm::Type* vt = val->getType();
		
		llvm::Value* buf = builder.CreateAlloca(llvm::ArrayType::get(type_i8, 64));
		
		llvm::StringRef vf = "%x";
		if(vt->isIntegerTy())
			vf = vt->getIntegerBitWidth() == 8 ? "%c" : "%d";
		else if(vt->isFloatingPointTy())
			vf = "%f";
		
		llvm::Value* fmt = builder.CreateGlobalString(vf);
		
		auto sprintf = module.getFunction("sprintf");
		builder.CreateCall(sprintf, {buf, fmt, val});
		
		return buf;
	}
	
	llvm::Value* llvm_module_t::cstr_from_bool(llvm::Function* func, llvm::IRBuilder<>& builder, llvm::Value* val)
	{
		llvm::BasicBlock* branch_true = llvm::BasicBlock::Create(context, "branch.true", func);
		llvm::BasicBlock* branch_false = llvm::BasicBlock::Create(context, "branch.false", func);
		llvm::BasicBlock* branch_end = llvm::BasicBlock::Create(context, "branch.end", func);
		
		builder.CreateCondBr(val, branch_true, branch_false);
		
		builder.SetInsertPoint(branch_true);
		auto val_true = builder.CreateGlobalStringPtr("true");
		builder.CreateBr(branch_end);
		
		builder.SetInsertPoint(branch_false);
		auto val_false = builder.CreateGlobalStringPtr("false");
		builder.CreateBr(branch_end);
		
		builder.SetInsertPoint(branch_end);
		llvm::PHINode* phi = builder.CreatePHI(type_cstr, 2);
		phi->addIncoming(val_true, branch_true);
		phi->addIncoming(val_false, branch_false);
		
		return phi;
	}
	
	llvm::Value* llvm_module_t::string_make(llvm::Function* func, llvm::IRBuilder<>& builder, const char* cstr)
	{
		auto str = this->make(func, builder, this->type_string);
		
		auto dataP = builder.CreateStructGEP(str, 0);
		auto dataV = builder.CreateGlobalString(cstr);
		builder.CreateStore(dataV, dataP);
		
		auto lengthP = builder.CreateStructGEP(str, 1);
		auto lengthV = builder.getInt32(std::strlen(cstr));
		builder.CreateStore(lengthV, lengthP);
		
		return str;
	}
	
	llvm::Value* llvm_module_t::string_from_cstr(llvm::Function* func, llvm::IRBuilder<>& builder, llvm::Value* cstr)
	{
		llvm::Value* str = this->make(func, builder, type_string);
		
		llvm::Value* dataP = builder.CreateStructGEP(str, 0);
		builder.CreateStore(cstr, dataP);
		
		auto lengthP = builder.CreateStructGEP(str, 1);
		auto lengthV = this->cstr_len(func, builder, cstr);
		builder.CreateStore(lengthV, lengthP);
		
		return str;
	}
	
	llvm::Value* llvm_module_t::string_from_value(llvm::Function* func, llvm::IRBuilder<>& builder, llvm::Value* val)
	{
		llvm::Type* vt = val->getType();
		
		if(vt == type_string_ptr)
			return val;
		
		llvm::Value* cstr = nullptr;
		if(vt == type_cstr)
			cstr = val;
		else if(vt == type_bool)
			cstr = this->cstr_from_bool(func, builder, val);
		else
			cstr = this->cstr_from_number(func, builder, val);
		
		return this->string_from_cstr(func, builder, cstr);
	}
	
	llvm::Value* llvm_module_t::string_get_char(llvm::Function* func, llvm::IRBuilder<>& builder, llvm::Value* str, llvm::Value* index)
	{
		auto dataPtr = builder.CreateStructGEP(str, 0);
		return builder.CreateGEP(dataPtr, index);
	}
	
	llvm::Value* llvm_module_t::string_concat(llvm::Function* func, llvm::IRBuilder<>& builder, llvm::Value* str1, llvm::Value* str2)
	{
		return nullptr;
	}
	
	llvm::Value* llvm_module_t::print(llvm::Function* func, llvm::IRBuilder<>& builder, const std::vector<llvm::Value*>& args)
	{
		llvm::Value* val = args[0];
		llvm::Value* fmt = builder.CreateGlobalString("%s ");
		llvm::Value* cstr = this->cstr_from_value(func, builder, val);
		auto pfn = module.getFunction("printf");
		llvm::Value* ret = builder.CreateCall(pfn, {fmt, cstr});
		
		return ret;
	}
_EndNamespace(eokas)
