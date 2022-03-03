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
	
	bool llvm_scope_t::addSymbol(const String& name, llvm_expr_t* expr)
	{
		bool ret = this->symbols.add(name, expr);
		if(ret)
		{
			expr->scope = this;
		}
		return ret;
	}
	
	llvm_expr_t* llvm_scope_t::getSymbol(const String& name, bool lookup)
	{
		if(lookup)
		{
			for (auto scope = this; scope != nullptr; scope = scope->parent)
			{
				auto symbol = scope->symbols.get(name);
				if(symbol != nullptr)
					return symbol;
			}
			return nullptr;
		}
		else
		{
			return this->symbols.get(name);
		}
	}
	
	bool llvm_scope_t::addType(const String& name, llvm_type_t* type)
	{
		bool ret = this->types.add(name, type);
		if(ret)
		{
			type->scope = this;
		}
		return ret;
	}
	
	llvm_type_t* llvm_scope_t::getType(const String& name, bool lookup)
	{
		if(lookup)
		{
			for (auto scope = this; scope != nullptr; scope = scope->parent)
			{
				auto type = scope->types.get(name);
				if(type != nullptr)
					return type;
			}
			return nullptr;
		}
		else
		{
			return this->types.get(name);
		}
	}
	
	llvm_type_t::llvm_type_t(llvm::LLVMContext& context, const String& name, llvm::Type* handle, llvm::Value* defval)
		: context(context), name(name), members(), handle(handle), defval(defval), scope(nullptr)
	{
	}
	
	llvm_type_t::~llvm_type_t() noexcept
	{
		_DeleteList(this->members);
	}
	
	bool llvm_type_t::extends(llvm_type_t* base, String& err)
	{
		for (const auto& baseMember: base->members)
		{
			if(this->get_member(baseMember->name) != nullptr)
			{
				err = String::format("The member named '%s' is already exists. \n", baseMember->name.cstr());
				return false;
			}
			this->add_member(baseMember);
		}
		return true;
	}
	
	llvm_type_t::member_t* llvm_type_t::add_member(const String& name, llvm_type_t* type, llvm_expr_t* value)
	{
		auto* m = new member_t();
		m->name = name;
		m->type = type;
		m->value = value;
		this->members.push_back(m);
		
		return m;
	}
	
	llvm_type_t::member_t* llvm_type_t::add_member(const member_t* other)
	{
		return this->add_member(other->name, other->type, other->value);
	}
	
	llvm_type_t::member_t* llvm_type_t::get_member(const String& name) const
	{
		for (auto& m: this->members)
		{
			if(m->name == name)
				return m;
		}
		return nullptr;
	}
	
	llvm_type_t::member_t* llvm_type_t::get_member(size_t index) const
	{
		if(index>=this->members.size())
			return nullptr;
		return this->members.at(index);
	}
	
	size_t llvm_type_t::get_member_index(const String& name) const
	{
		for (size_t index = 0; index<this->members.size(); index++)
		{
			if(this->members.at(index)->name == name)
				return index;
		}
		return -1;
	}
	
	void llvm_type_t::resolve(bool force)
	{
		if(force || this->handle == nullptr)
		{
			llvm::StructType* type = llvm::StructType::create(context, this->name.cstr());
			std::vector<llvm::Type*> body;
			for (auto& member: this->members)
			{
				member->type->resolve();
				body.push_back(member->type->handle);
			}
			type->setBody(body);
			this->handle = type;
		}
		
		if(force || this->defval == nullptr)
		{
			auto structType = llvm::cast<llvm::StructType>(this->handle);
			auto structRefType = structType->getPointerTo();
			this->defval = llvm::ConstantPointerNull::get(structRefType);
		}
	}
	
	void llvm_type_t::ref(llvm_type_t* element_type)
	{
		auto handle = llvm::PointerType::get(element_type->handle, 0);
		auto defval = llvm::ConstantPointerNull::get(handle);
		this->handle = handle;
		this->defval = defval;
	}
	
	llvm_expr_t::llvm_expr_t(llvm::Value* value, llvm::Type* type)
	{
		this->value = value;
		if(type != nullptr)
		{
			this->type = type;
		}
		else
		{
			this->type = value->getType();
		}
	}
	
	llvm_expr_t::~llvm_expr_t()
	{
		this->value = nullptr;
		this->type = nullptr;
	}
	
	bool llvm_expr_t::is_symbol() const
	{
		if(this->type == nullptr || this->value == nullptr)
			return false;
		auto vt = this->value->getType();
		return vt->isPointerTy() && vt->getPointerElementType() == this->type;
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
		type_void = this->new_type("void", llvm::Type::getVoidTy(context), llvm::ConstantInt::get(context, llvm::APInt(8, 0)));
		type_i8 = this->new_type("i8", llvm::Type::getInt8Ty(context), llvm::ConstantInt::get(context, llvm::APInt(8, 0)));
		type_i16 = this->new_type("i16", llvm::Type::getInt16Ty(context), llvm::ConstantInt::get(context, llvm::APInt(16, 0)));
		type_i32 = this->new_type("i32", llvm::Type::getInt32Ty(context), llvm::ConstantInt::get(context, llvm::APInt(32, 0)));
		type_i64 = this->new_type("i64", llvm::Type::getInt64Ty(context), llvm::ConstantInt::get(context, llvm::APInt(64, 0)));
		type_u8 = this->new_type("u8", llvm::Type::getInt8Ty(context), llvm::ConstantInt::get(context, llvm::APInt(8, 0)));
		type_u16 = this->new_type("u16", llvm::Type::getInt16Ty(context), llvm::ConstantInt::get(context, llvm::APInt(16, 0)));
		type_u32 = this->new_type("u32", llvm::Type::getInt32Ty(context), llvm::ConstantInt::get(context, llvm::APInt(32, 0)));
		type_u64 = this->new_type("u64", llvm::Type::getInt64Ty(context), llvm::ConstantInt::get(context, llvm::APInt(64, 0)));
		type_f32 = this->new_type("f32", llvm::Type::getFloatTy(context), llvm::ConstantFP::get(context, llvm::APFloat(0.0f)));
		type_f64 = this->new_type("f64", llvm::Type::getDoubleTy(context), llvm::ConstantFP::get(context, llvm::APFloat(0.0)));
		type_bool = this->new_type("bool", llvm::Type::getInt1Ty(context), llvm::ConstantInt::get(context, llvm::APInt(1, 0)));
		
		type_i8_ref = llvm_typedef_ref_t::define_type(this, type_i8);
		
		type_string = llvm_typedef_string_t::define_type(this);
		type_string_ref = llvm_typedef_ref_t::define_type(this, type_string);
		
		type_enum = this->new_type("enum", nullptr, nullptr);
		type_enum->add_member("value", type_i32, this->new_expr(type_i32->defval));
		type_enum->resolve();
		
		type_enum_ref = llvm_typedef_ref_t::define_type(this, type_enum);
		
		this->declare_func_printf();
		this->declare_func_sprintf();
		this->declare_func_malloc();
		this->declare_func_free();
		
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
		this->root->addType("string", type_string_ref);
		
		this->root->addSymbol("print", this->new_expr(this->define_func_print()));
	}
	
	llvm_module_t::~llvm_module_t() noexcept
	{
		_DeleteList(this->exprs);
		_DeleteList(this->types);
		_DeletePointer(root);
	}
	
	llvm_expr_t* llvm_module_t::new_expr(llvm::Value* value, llvm::Type* type)
	{
		llvm_expr_t* expr = nullptr;
		if(value->getType()->isPointerTy() && value->getType()->getPointerElementType()->isFunctionTy())
		{
			expr = new llvm_func_t(value, type);
		}
		else
		{
			expr = new llvm_expr_t(value, type);
		}
		
		this->exprs.push_back(expr);
		
		return expr;
	}
	
	llvm_type_t* llvm_module_t::new_type(const String& name, llvm::Type* handle, llvm::Value* defval)
	{
		auto type = new llvm_type_t(context, name, handle, defval);
		this->types.push_back(type);
		return type;
	}
	
	llvm_type_t* llvm_module_t::new_type(const String& name, llvm_type_t* base)
	{
		auto type = this->new_type(name, nullptr, nullptr);
		for (const auto& baseMember: base->members)
		{
			type->add_member(baseMember);
		}
		type->resolve();
		return type;
	}
	
	llvm_type_t* llvm_module_t::get_type(const String& name)
	{
		auto iter = this->types.begin();
		while(iter != this->types.end())
		{
			auto type = *iter;
			if(type->name == name)
				return type;
			++ iter;
		}
		return nullptr;
	}
	
	llvm_type_t* llvm_module_t::get_type(llvm::Type* handle)
	{
		auto iter = this->types.begin();
		while(iter != this->types.end())
		{
			auto type = *iter;
			if(type->handle == handle)
				return type;
			++iter;
		}
		return nullptr;
	}
	
	llvm::Function* llvm_module_t::declare_func_printf()
	{
		String name = "printf";
		llvm::Type* ret = type_i32->handle;
		std::vector<llvm::Type*> args = {type_i8_ref->handle};
		bool varg = true;
		
		return llvm_model_t::declare_func(module, name, ret, args, varg);
	}
	
	llvm::Function* llvm_module_t::declare_func_sprintf()
	{
		String name = "sprintf";
		llvm::Type* ret = type_i32->handle;
		std::vector<llvm::Type*> args = {type_i8_ref->handle, type_i8_ref->handle};
		bool varg = true;
		
		return llvm_model_t::declare_func(module, name, ret, args, varg);
	}
	
	llvm::Function* llvm_module_t::declare_func_malloc()
	{
		String name = "malloc";
		llvm::Type* ret = type_i8_ref->handle;
		std::vector<llvm::Type*> args = {type_i64->handle};
		bool varg = false;
		
		return llvm_model_t::declare_func(module, name, ret, args, varg);
	}
	
	llvm::Function* llvm_module_t::declare_func_free()
	{
		String name = "free";
		llvm::Type* ret = type_void->handle;
		std::vector<llvm::Type*> args = {type_i8_ref->handle};
		bool varg = false;
		
		return llvm_model_t::declare_func(module, name, ret, args, varg);
	}
	
	llvm::Function* llvm_module_t::define_func_print()
	{
		String name = "print";
		llvm::Type* ret = type_i32->handle;
		std::vector<llvm::Type*> args = {type_string_ref->handle};
		bool varg = false;
		
		return llvm_model_t::define_func(module, name, ret, args, varg, [&](llvm::LLVMContext& context, llvm::Module& module, llvm::Function* func, llvm::IRBuilder<>& builder)->void
		{
			llvm::BasicBlock* entry = llvm::BasicBlock::Create(context, "entry", func);
			builder.SetInsertPoint(entry);
			
			llvm::Value* arg0 = func->getArg(0);
			llvm::Value* cstr = this->string_to_cstr(func, builder, arg0);
			llvm::Value* retval = this->print(func, builder, {cstr});
			
			builder.CreateRet(retval);
		});
	}
	
	llvm::Value* llvm_module_t::make(llvm::Function* func, llvm::IRBuilder<>& builder, llvm::Type* type)
	{
		auto mallocF = module.getFunction("malloc");
		llvm::Constant* len = llvm::ConstantExpr::getSizeOf(type);
		llvm::Value* ptr = builder.CreateCall(mallocF, {len});
		llvm::Value* val = builder.CreateBitCast(ptr, type->getPointerTo());
		return val;
	}
	
	void llvm_module_t::free(llvm::Function* func, llvm::IRBuilder<>& builder, llvm::Value* ptr)
	{
		auto freeF = module.getFunction("free");
		builder.CreateCall(freeF, {ptr});
	}
	
	llvm::Value* llvm_module_t::make_string(llvm::Function* func, llvm::IRBuilder<>& builder, const char* cstr)
	{
		auto str = this->make(func, builder, this->type_string->handle);
		auto memberPtr = builder.CreateStructGEP(str, 0);
		auto memberVal = builder.CreateGlobalString(cstr);
		builder.CreateStore(memberVal, memberPtr);
		return str;
	}
	
	llvm::Value* llvm_module_t::string_to_cstr(llvm::Function* func, llvm::IRBuilder<>& builder, llvm::Value* val)
	{
		llvm::Value* ptr = builder.CreateStructGEP(type_string->handle, val, 0);
		llvm::Value* cstr = builder.CreateLoad(ptr);
		return cstr;
	}
	
	llvm::Value* llvm_module_t::bool_to_cstr(llvm::Function* func, llvm::IRBuilder<>& builder, llvm::Value* val)
	{
		llvm::BasicBlock* branch_begin = llvm::BasicBlock::Create(context, "branch.begin", func);
		llvm::BasicBlock* branch_true = llvm::BasicBlock::Create(context, "branch.true", func);
		llvm::BasicBlock* branch_false = llvm::BasicBlock::Create(context, "branch.false", func);
		llvm::BasicBlock* branch_end = llvm::BasicBlock::Create(context, "branch.end", func);
		
		builder.CreateBr(branch_begin);
		
		builder.SetInsertPoint(branch_begin);
		builder.CreateCondBr(val, branch_true, branch_false);
		
		builder.SetInsertPoint(branch_true);
		auto val_true = builder.CreateGlobalStringPtr("true");
		builder.CreateBr(branch_end);
		
		builder.SetInsertPoint(branch_false);
		auto val_false = builder.CreateGlobalStringPtr("false");
		builder.CreateBr(branch_end);
		
		builder.SetInsertPoint(branch_end);
		
		llvm::PHINode* phi = builder.CreatePHI(type_i8_ref->handle, 2);
		phi->addIncoming(val_true, branch_true);
		phi->addIncoming(val_false, branch_false);
		
		return phi;
	}
	
	llvm::Value* llvm_module_t::number_to_cstr(llvm::Function* func, llvm::IRBuilder<>& builder, llvm::Value* val)
	{
		llvm::Type* vt = val->getType();
		
		llvm::Value* buf = builder.CreateAlloca(llvm::ArrayType::get(type_i8->handle, 64));
		
		llvm::StringRef vf = "%x";
		if(vt->isIntegerTy())
			vf = "%d";
		else if(vt->isFloatingPointTy())
			vf = "%f";
		
		llvm::Value* fmt = builder.CreateGlobalString(vf);
		
		auto sprintf = module.getFunction("sprintf");
		builder.CreateCall(sprintf, {buf, fmt, val});
		
		return buf;
	}
	
	llvm::Value* llvm_module_t::enum_to_cstr(llvm::Function* func, llvm::IRBuilder<>& builder, llvm::Value* val)
	{
		llvm::Value* ptr = builder.CreateStructGEP(type_enum->handle, val, 0);
		llvm::Value* code = builder.CreateLoad(ptr);
		
		llvm::Value* buf = builder.CreateAlloca(llvm::ArrayType::get(type_i8->handle, 64));
		llvm::Value* fmt = builder.CreateGlobalString("%d");
		
		auto sprintf = module.getFunction("sprintf");
		builder.CreateCall(sprintf, {buf, fmt, code});
		
		return buf;
	}
	
	llvm::Value* llvm_module_t::value_to_cstr(llvm::Function* func, llvm::IRBuilder<>& builder, llvm::Value* val)
	{
		llvm::Type* vt = val->getType();
		
		if(vt == type_string_ref->handle)
			return this->string_to_cstr(func, builder, val);
		if(vt == type_enum_ref->handle)
			return this->enum_to_cstr(func, builder, val);
		
		if(vt == type_i8_ref->handle)
			return val;
		
		if(vt->isIntegerTy(1))
			return this->bool_to_cstr(func, builder, val);
		
		return this->number_to_cstr(func, builder, val);
	}
	
	llvm::Value* llvm_module_t::cstr_to_string(llvm::Function* func, llvm::IRBuilder<>& builder, llvm::Value* val)
	{
		llvm::Value* str = this->make(func, builder, type_string->handle);
		llvm::Value* ptr = builder.CreateStructGEP(type_string->handle, str, 0);
		builder.CreateStore(val, ptr);
		return str;
	}

	llvm::Value* llvm_module_t::value_to_string(llvm::Function* func, llvm::IRBuilder<>& builder, llvm::Value* val)
	{
		llvm::Type* vt = val->getType();
		
		if(vt == type_string_ref->handle)
			return val;
		
		if(vt == type_i8_ref->handle)
			return this->cstr_to_string(func, builder, val);
		
		if(vt->isIntegerTy(1))
		{
			llvm::Value* cstr = this->bool_to_cstr(func, builder, val);
			return this->cstr_to_string(func, builder, cstr);
		}
		
		if(vt == type_enum_ref->handle)
		{
			llvm::Value* cstr = this->enum_to_cstr(func, builder, val);
			return this->cstr_to_string(func, builder, cstr);
		}
		
		llvm::Value* cstr = this->number_to_cstr(func, builder, val);
		return this->cstr_to_string(func, builder, cstr);
	}
	
	llvm::Value* llvm_module_t::print(llvm::Function* func, llvm::IRBuilder<>& builder, const std::vector<llvm::Value*>& args)
	{
		llvm::Value* val = args[0];
		llvm::Value* fmt = builder.CreateGlobalString("%s");
		llvm::Value* cstr = this->value_to_cstr(func, builder, val);
		auto pfn = module.getFunction("printf");
		llvm::Value* ret = builder.CreateCall(pfn, {fmt, cstr});
		
		return ret;
	}
_EndNamespace(eokas)
