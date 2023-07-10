
#include "./builder.h"

namespace eokas
{
	llvm_basic_builder_t::llvm_basic_builder_t(llvm::LLVMContext& context)
		:context(context)
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
		type_void_ptr = type_void->getPointerTo();
	}
	
	llvm_basic_builder_t::~llvm_basic_builder_t()
	{ }
	
	String llvm_basic_builder_t::get_type_name(llvm::Type* type)
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
	
	llvm::Value* llvm_basic_builder_t::get_default_value(llvm::Type* type)
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
	
	llvm_module_builder_t::llvm_module_builder_t(llvm::LLVMContext& context, const String& name)
		: llvm_basic_builder_t(context)
		, module(name.cstr(), context)
		, builder(context)
		, types()
		, funcs()
	{ }
	
	llvm_module_builder_t::~llvm_module_builder_t()
	{
		_DeleteMap(types);
		_DeleteList(funcs);
	}
	
	void llvm_module_builder_t::resolve()
	{
		for(auto& type : this->types)
		{
			type.second->resolve();
		}
	}
	
	llvm_type_builder_t* llvm_module_builder_t::add_type(const String& name, llvm_type_builder_t* type)
	{
		if(type == nullptr || &type->module != this)
			return nullptr;
		if(this->get_type(name) != nullptr)
			return nullptr;
		this->types.insert(std::make_pair(name, type));
		return type;
	}
	
	llvm_type_builder_t* llvm_module_builder_t::get_type(const String& name)
	{
		auto iter = this->types.find(name);
		if(iter == this->types.end())
			return nullptr;
		return iter->second;
	}
	
	llvm_func_builder_t* llvm_module_builder_t::add_func(const String& name, llvm::Type* ret, const std::vector<llvm::Type*>& args, bool varg)
	{
		if(name == "" || ret == nullptr)
			return nullptr;
		if(this->get_func(name) != nullptr)
			return nullptr;
		auto func = new llvm_func_builder_t(this, nullptr, name, ret, args, varg);
		this->funcs.insert(std::make_pair(name, func));
		return func;
	}
	
	llvm_func_builder_t* llvm_module_builder_t::get_func(const String& name)
	{
		auto iter = this->funcs.find(name);
		if(iter == this->funcs.end())
			return nullptr;
		return iter->second;
	}
	
	llvm::Function* llvm_module_builder_t::define_func(const String& name, llvm::Type* ret, const std::vector<llvm::Type*>& args, bool varg, const llvm_code_delegate_t& body)
	{
		auto& context = this->module.getContext();
		
		llvm::AttributeList attrs;
		
		auto funcType = llvm::FunctionType::get(ret, args, varg);
		auto funcValue = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, name.cstr(), this->module);
		funcValue->setCallingConv(llvm::CallingConv::C);
		funcValue->setAttributes(attrs);
		
		llvm::IRBuilder<> builder(context);
		
		body(context, module, funcValue, builder);
		
		return funcValue;
	}
	
	llvm_type_builder_t::llvm_type_builder_t(llvm_module_builder_t& module, const String& name)
		: llvm_basic_builder_t(module.context)
		, module(module)
		, name(name)
		, handle(nullptr)
		, members()
	{
		this->handle = llvm::StructType::create(context, name.cstr());
	}
	
	void llvm_type_builder_t::resolve()
	{
		std::vector<llvm::Type*> body;
		for (auto& member: this->members)
		{
			body.push_back(member.type);
		}
		this->handle->setBody(body);
	}
	
	bool llvm_type_builder_t::extends(const String& base)
	{
		return this->extends(module.get_type(base));
	}
	
	bool llvm_type_builder_t::extends(llvm_type_builder_t* base)
	{
		if(base == nullptr)
			return false;
		this->add_member("base", base->handle);
		for(auto& m : base->members)
		{
			if(this->is_final_member(m.name))
				continue;
			if(this->add_member(&m) == nullptr)
				return false;
		}
		return true;
	}
	
	llvm_type_builder_t::member_t* llvm_type_builder_t::add_member(const String& name, llvm::Type* type, llvm::Value* value)
	{
		if(this->get_member(name) != nullptr)
			return nullptr;
		if(type == nullptr && value == nullptr)
			return nullptr;
		
		member_t& m = this->members.emplace_back();
		m.name = name;
		m.type = type;
		m.value = value;
		
		if(type == nullptr)
		{
			m.type = value->getType();
		}
		
		return &m;
	}
	
	llvm_type_builder_t::member_t* llvm_type_builder_t::add_member(const member_t* other)
	{
		return this->add_member(other->name, other->type, other->value);
	}
	
	llvm_type_builder_t::member_t* llvm_type_builder_t::get_member(const String& name)
	{
		for(auto& m : this->members)
		{
			if(m.name == name)
				return &m;
		}
		return nullptr;
	}
	
	llvm_type_builder_t::member_t* llvm_type_builder_t::get_member(size_t index)
	{
		if(index>=this->members.size())
			return nullptr;
		member_t& m = this->members.at(index);
		return &m;
	}
	
	size_t llvm_type_builder_t::get_member_index(const String& name) const
	{
		for (size_t index = 0; index < this->members.size(); index++)
		{
			if(this->members.at(index).name == name)
				return index;
		}
		return -1;
	}
	
	bool llvm_type_builder_t::is_final_member(const String& name) const
	{
		return name == "base";
	}
	
	llvm_func_builder_t::llvm_func_builder_t(
		llvm_module_builder_t* module,
		llvm_type_builder_t* owner,
		const String& name,
		llvm::Type* retT,
		const std::vector<llvm::Type*> argsT,
		bool varg)
		: llvm_basic_builder_t(module->context)
		, module(module)
		, owner(owner)
		, IR(module->context)
	{
		this->type = llvm::FunctionType::get(retT, argsT, varg);
		this->handle = llvm::Function::Create(
			this->type,
			llvm::Function::ExternalLinkage,
			name.cstr(),
			&module->module);
			
		llvm::AttributeList attrs;
		this->handle->setAttributes(attrs);
		this->handle->setCallingConv(llvm::CallingConv::C);
	}
	
	void llvm_func_builder_t::resolve()
	{ }
	
	/**
	 * For ref-types, transform multi-level pointer to one-level pointer.
	 * For val-types, transform multi-level pointer to real value.
	 * */
	llvm::Value* llvm_func_builder_t::get_value(llvm::Value* value)
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
			value = IR.CreateLoad(value);
			type = value->getType();
		}
		
		return value;
	}
	
	/**
	 * Transform the multi-level pointer value to one-level pointer type value.
	 * Ignores literal values.
	 * */
	llvm::Value* llvm_func_builder_t::ref_value(llvm::Value* value)
	{
		llvm::Type* type = value->getType();
		
		while (type->isPointerTy() && type->getPointerElementType()->isPointerTy())
		{
			value = IR.CreateLoad(value);
			type = value->getType();
		}
		
		return value;
	}
	
	llvm::BasicBlock* llvm_func_builder_t::add_basic_block(const String& name)
	{
		llvm::BasicBlock* bb = llvm::BasicBlock::Create(context, name.cstr(), this->handle);
		return bb;
	}
	
	void llvm_func_builder_t::add_tail_ret()
	{
		auto& lastOp = IR.GetInsertBlock()->back();
		if(!lastOp.isTerminator())
		{
			auto retT = this->handle->getReturnType();
			if(retT->isVoidTy())
				IR.CreateRetVoid();
			else
				IR.CreateRet(this->get_default_value(retT));
		}
	}
	
	bool llvm_func_builder_t::is_array_type(llvm::Type* type)
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
	
	llvm::Value* llvm_func_builder_t::make(llvm::Type* type)
	{
		auto mallocF = module->module.getFunction("malloc");
		llvm::Constant* len = llvm::ConstantExpr::getSizeOf(type);
		llvm::Value* ptr = IR.CreateCall(mallocF, {len});
		llvm::Value* val = IR.CreateBitCast(ptr, type->getPointerTo());
		return val;
	}
	
	llvm::Value* llvm_func_builder_t::make(llvm::Type* type, llvm::Value* count)
	{
		auto mallocF = module->module.getFunction("malloc");
		llvm::Constant* stride = llvm::ConstantExpr::getSizeOf(type);
		llvm::Value* len = IR.CreateMul(stride, count);
		llvm::Value* ptr = IR.CreateCall(mallocF, {len});
		llvm::Value* val = IR.CreateBitCast(ptr, type->getPointerTo());
		return val;
	}
	
	llvm::Value* llvm_func_builder_t::make(llvm_type_builder_t* type)
	{
		auto ptr = this->make(type->handle);
		for(size_t i = 0; i < type->members.size(); i++)
		{
			auto mem = type->get_member(i);
			auto p = IR.CreateStructGEP(ptr, i);
			auto v = mem->value;
			IR.CreateStore(v, p);
		}
		return ptr;
	}
	
	void llvm_func_builder_t::free(llvm::Value* ptr)
	{
		auto freeF = module->module.getFunction("free");
		IR.CreateCall(freeF, {ptr});
	}
	
	llvm::Value* llvm_func_builder_t::array_set(llvm::Value* array, const llvm::ArrayRef<llvm::Value*>& elements)
	{
		auto elementT = elements[0]->getType();
		auto arrayT = llvm::ArrayType::get(elementT, elements.size());
		
		auto* dataP = IR.CreateStructGEP(array, 0);
		auto* dataV = this->make(arrayT);
		for (size_t i = 0; i < elements.size(); i++)
		{
			auto elementP = IR.CreateConstGEP2_64(dataV, 0, i);
			auto elementV = elements[i];
			IR.CreateStore(elementV, elementP);
		}
		IR.CreateStore(dataV, dataP);
		
		auto countP = IR.CreateStructGEP(array, 1);
		auto countV = IR.getInt64(elements.size());
		IR.CreateStore(countV, countP);
		
		return array;
	}
	
	llvm::Value* llvm_func_builder_t::array_get(llvm::Value* array, llvm::Value* index)
	{
		auto dataP = IR.CreateStructGEP(array, 0);
		auto dataV = IR.CreateLoad(dataP);
		auto val = IR.CreateInBoundsGEP(dataV, {IR.getInt64(0), index});
		return val;
	}
	
	llvm::Value* llvm_func_builder_t::cstr_len(llvm::Value* val)
	{
		llvm::Value* cstr = this->cstr_from_value(val);
		auto pfn = module->module.getFunction("strlen");
		llvm::Value* ret = IR.CreateCall(pfn, {cstr});
		
		return ret;
	}
	
	llvm::Value* llvm_func_builder_t::cstr_from_value(llvm::Value* val)
	{
		llvm::Type* vt = val->getType();
		
		if(vt == type_cstr)
			return val;
		
		if(vt == type_string_ptr)
			return this->cstr_from_string(val);
		
		if(vt == type_bool)
			return this->cstr_from_bool(val);
		
		return this->cstr_from_number(val);
	}
	
	llvm::Value* llvm_func_builder_t::cstr_from_string(llvm::Value* str)
	{
		llvm::Value* ptr = IR.CreateStructGEP(type_string, str, 0);
		llvm::Value* cstr = IR.CreateLoad(ptr);
		return cstr;
	}
	
	llvm::Value* llvm_func_builder_t::cstr_from_number(llvm::Value* val)
	{
		llvm::Type* vt = val->getType();
		
		llvm::Value* buf = IR.CreateAlloca(llvm::ArrayType::get(type_i8, 64));
		
		llvm::StringRef vf = "%x";
		if(vt->isIntegerTy())
			vf = vt->getIntegerBitWidth() == 8 ? "%c" : "%d";
		else if(vt->isFloatingPointTy())
			vf = "%f";
		
		llvm::Value* fmt = IR.CreateGlobalString(vf);
		
		auto sprintf = module->module.getFunction("sprintf");
		IR.CreateCall(sprintf, {buf, fmt, val});
		
		return buf;
	}
	
	llvm::Value* llvm_func_builder_t::cstr_from_bool(llvm::Value* val)
	{
		llvm::BasicBlock* branch_true = this->add_basic_block("branch.true");
		llvm::BasicBlock* branch_false = this->add_basic_block("branch.false");
		llvm::BasicBlock* branch_end = this->add_basic_block("branch.end");
		
		IR.CreateCondBr(val, branch_true, branch_false);
		
		IR.SetInsertPoint(branch_true);
		auto val_true = IR.CreateGlobalStringPtr("true");
		IR.CreateBr(branch_end);
		
		IR.SetInsertPoint(branch_false);
		auto val_false = IR.CreateGlobalStringPtr("false");
		IR.CreateBr(branch_end);
		
		IR.SetInsertPoint(branch_end);
		llvm::PHINode* phi = IR.CreatePHI(type_cstr, 2);
		phi->addIncoming(val_true, branch_true);
		phi->addIncoming(val_false, branch_false);
		
		return phi;
	}
	
	llvm::Value* llvm_func_builder_t::string_make(const char* cstr)
	{
		auto str = this->make(type_string);
		
		auto dataP = IR.CreateStructGEP(str, 0);
		auto dataV = IR.CreateGlobalString(cstr);
		IR.CreateStore(dataV, dataP);
		
		auto lengthP = IR.CreateStructGEP(str, 1);
		auto lengthV = IR.getInt32(std::strlen(cstr));
		IR.CreateStore(lengthV, lengthP);
		
		return str;
	}
	
	llvm::Value* llvm_func_builder_t::string_from_cstr(llvm::Value* cstr)
	{
		llvm::Value* str = this->make(type_string);
		
		llvm::Value* dataP = IR.CreateStructGEP(str, 0);
		IR.CreateStore(cstr, dataP);
		
		auto lengthP = IR.CreateStructGEP(str, 1);
		auto lengthV = this->cstr_len(cstr);
		IR.CreateStore(lengthV, lengthP);
		
		return str;
	}
	
	llvm::Value* llvm_func_builder_t::string_from_value(llvm::Value* val)
	{
		llvm::Type* vt = val->getType();
		
		if(vt == type_string_ptr)
			return val;
		
		llvm::Value* cstr = nullptr;
		if(vt == type_cstr)
			cstr = val;
		else if(vt == type_bool)
			cstr = this->cstr_from_bool(val);
		else
			cstr = this->cstr_from_number(val);
		
		return this->string_from_cstr(cstr);
	}
	
	llvm::Value* llvm_func_builder_t::string_get_char(llvm::Value* str, llvm::Value* index)
	{
		auto dataPtr = IR.CreateStructGEP(str, 0);
		return IR.CreateGEP(dataPtr, index);
	}
	
	llvm::Value* llvm_func_builder_t::string_concat(llvm::Value* str1, llvm::Value* str2)
	{
		return nullptr;
	}
	
	llvm::Value* llvm_func_builder_t::print(const std::vector<llvm::Value*>& args)
	{
		llvm::Value* val = args[0];
		llvm::Value* fmt = IR.CreateGlobalString("%s ");
		llvm::Value* cstr = this->cstr_from_value(val);
		auto pfn = module->module.getFunction("printf");
		llvm::Value* ret = IR.CreateCall(pfn, {fmt, cstr});
		
		return ret;
	}
	
}
