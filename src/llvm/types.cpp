#include "./types.h"

namespace eokas
{
    LlvmTypeAny::LlvmTypeAny(Category category, llvm::Type *type)
        : mCategory(category), mHandle(type)
    { }

    LlvmTypeAny::~LlvmTypeAny()
    {}

    bool LlvmTypeAny::operator==(const LlvmTypeAny &other)
    {
        return mCategory == other.mCategory && mHandle == other.mHandle;
    }

    bool LlvmTypeAny::operator!=(const LlvmTypeAny &other)
    {
        return ! (*this == other);
    }

    LlvmTypeAny::Category LlvmTypeAny::getCategory()
    {
        return mCategory;
    }

    llvm::Type* LlvmTypeAny::getHandle()
    {
        return mHandle;
    }

    llvm::Value* LlvmTypeAny::getDefault()
    {
        return mDefault;
    }

    LlvmTypeInteger::LlvmTypeInteger(llvm::Type* type)
        : LlvmTypeAny(Category::Integer, type)
        , mBits(32)
        , mUnsigned(false)
    {}

    u32_t LlvmTypeInteger::getBits()
    {
        return mBits;
    }

    void LlvmTypeInteger::setBits(u32_t bits)
    {
        mBits = bits;
    }

    bool LlvmTypeInteger::isUnsigned()
    {
        return mUnsigned;
    }

    void LlvmTypeInteger::setUnsigned(bool value)
    {
        mUnsigned = value;
    }

    LlvmTypeFloat::LlvmTypeFloat(llvm::Type *type)
        : LlvmTypeAny(Category::Float, type)
    {}

    u32_t LlvmTypeFloat::getBits()
    {
        return mBits;
    }

    void LlvmTypeFloat::setBits(u32_t bits)
    {
        mBits = bits;
    }

    LlvmTypePointer::LlvmTypePointer(llvm::Type* type)
        : LlvmTypeAny(Category::Pointer, type)
    { }

    llvm::Type* LlvmTypePointer::getElementType()
    {
        return mHandle->getPointerElementType();
    }

    LlvmTypeArray::LlvmTypeArray(llvm::Type* type)
        : LlvmTypeAny(Category::Array, type)
    {}

    llvm::Type* LlvmTypeArray::getElementType()
    {
        return mHandle->getArrayElementType();
    }

    LlvmTypeFunction::LlvmTypeFunction(llvm::Type* type)
        : LlvmTypeAny(Category::Function, type)
    {}

	LlvmTypeStruct::LlvmTypeStruct(llvm::Type* type)
		: LlvmTypeAny(Category::Struct, type)
		, mMembers()
		, mResolved(false)
	{}
	
	LlvmTypeStruct::~LlvmTypeStruct()
	{
		_DeleteList(mMembers);
	}
	
	LlvmTypeStruct::Member* LlvmTypeStruct::addMember(const String& name, LlvmTypeAny* type, llvm::Value* value)
	{
		if(this->getMember(name) != nullptr)
			return nullptr;
		if(type == nullptr && value == nullptr)
			return nullptr;
		
		auto* m = new Member();
		m->name = name;
		m->type = type;
		m->value = value;
		
		mMembers.push_back(m);
		mResolved = false;
		
		return m;
	}
	
	LlvmTypeStruct::Member* LlvmTypeStruct::addMember(const Member* other)
	{
		return this->addMember(other->name, other->type, other->value);
	}
	
	LlvmTypeStruct::Member* LlvmTypeStruct::getMember(const String& name)
	{
		for(auto& m : mMembers)
		{
			if(m->name == name)
				return m;
		}
		return nullptr;
	}
	
	LlvmTypeStruct::Member* LlvmTypeStruct::getMember(size_t index)
	{
		if(index >= mMembers.size())
			return nullptr;
		auto* m = mMembers.at(index);
		return m;
	}
	
	size_t LlvmTypeStruct::getMemberIndex(const String& name) const
	{
		for (size_t index = 0; index < mMembers.size(); index++)
		{
			if(mMembers.at(index)->name == name)
				return index;
		}
		return -1;
	}
	
	void LlvmTypeStruct::resolve()
	{
		std::vector<llvm::Type*> body;
		for (auto& m: mMembers)
		{
			body.push_back(m->type->getHandle());
		}
		
		auto* structT = llvm::cast<llvm::StructType>(mHandle);
		structT->setBody(body);
	}
	
	bool LlvmTypeStruct::isResolved() const
	{
		return mResolved;
	}
	
	LlvmTypeManager::LlvmTypeManager(llvm::LLVMContext& context)
		: mContext(context)
		, mTypes()
	{
		ty_void = llvm::Type::getVoidTy(context);
		ty_i8 = llvm::Type::getInt8Ty(context);
		ty_i16 = llvm::Type::getInt16Ty(context);
		ty_i32 = llvm::Type::getInt32Ty(context);
		ty_i64 = llvm::Type::getInt64Ty(context);
		ty_u8 = llvm::Type::getInt8Ty(context);
		ty_u16 = llvm::Type::getInt16Ty(context);
		ty_u32 = llvm::Type::getInt32Ty(context);
		ty_u64 = llvm::Type::getInt64Ty(context);
		ty_f32 = llvm::Type::getFloatTy(context);
		ty_f64 = llvm::Type::getDoubleTy(context);
		ty_bool = llvm::Type::getInt1Ty(context);
		ty_ptr_void = ty_void->getPointerTo();
		ty_ptr_u8 = ty_u8->getPointerTo();
	}
	
	LlvmTypeManager::~LlvmTypeManager()
	{
		_DeleteList(mTypes)
	}
	
	LlvmTypeInteger* LlvmTypeManager::integerType(u32_t bits, bool isUnsigned)
	{
		llvm::Type* llvmTy = nullptr;
		if(isUnsigned)
		{
			if(bits == 8) llvmTy = ty_u8;
			else if(bits == 16) llvmTy = ty_u16;
			else if(bits == 32) llvmTy == ty_u32;
			else if(bits == 64) llvmTy == ty_u64;
		}
		else
		{
			if(bits == 8) llvmTy = ty_i8;
			else if(bits == 16) llvmTy = ty_i16;
			else if(bits == 32) llvmTy == ty_i32;
			else if(bits == 64) llvmTy == ty_i64;
		}
		if(llvmTy == nullptr)
			return nullptr;
		auto type = new LlvmTypeInteger(llvmTy);
		mTypes.push_back(type);
		return type;
	}
	
	LlvmTypeFloat* LlvmTypeManager::floatType(u32_t bits)
	{
		llvm::Type* llvmTy = nullptr;
		if(bits == 32) llvmTy = ty_f32;
		else if(bits == 64) llvmTy = ty_f64;
		if(llvmTy == nullptr)
			return nullptr;
		auto type = new LlvmTypeFloat(llvmTy);
		mTypes.push_back(type);
		return type;
	}
	
	LlvmTypePointer* LlvmTypeManager::pointerType(LlvmTypeAny* elementType)
	{
		llvm::Type* llvmTy = llvm::PointerType::get(elementType->getHandle(), 0);
		auto type = new LlvmTypePointer(llvmTy);
		mTypes.push_back(type);
		return type;
	}
	
	LlvmTypeArray* LlvmTypeManager::arrayType(LlvmTypeAny* elementType)
	{
		llvm::Type* llvmTy = llvm::ArrayType::get(elementType->getHandle(), 0);
		auto type = new LlvmTypeArray(llvmTy);
		mTypes.push_back(type);
		return type;
	}
	
	LlvmTypeFunction* LlvmTypeManager::functionType(LlvmTypeAny* retType, const std::vector<LlvmTypeAny*>& argsTypes)
	{
		llvm::Type* retT = retType->getHandle();
		std::vector<llvm::Type*> argsT;
		for(auto& arg : argsTypes)
		{
			argsT.push_back(arg->getHandle());
		}
		llvm::Type* llvmTy = llvm::FunctionType::get(retT, argsT, false);
		auto type = new LlvmTypeFunction(llvmTy);
		mTypes.push_back(type);
		return type;
	}
	
	LlvmTypeStruct* LlvmTypeManager::structType(const String& name)
	{
		llvm::Type* llvmTy = llvm::StructType::create(mContext, name.cstr());
		auto type = new LlvmTypeStruct(llvmTy);
		mTypes.push_back(type);
		return type;
	}
	
	LlvmTypeInteger* LlvmTypeManager::castToIntegerType(LlvmTypeAny* type) const
	{
		return type->getCategory() == LlvmTypeAny::Category::Integer
			? dynamic_cast<LlvmTypeInteger*>(type)
			: nullptr;
	}
	
	LlvmTypeFloat* LlvmTypeManager::castToFloatType(LlvmTypeAny* type) const
	{
		return type->getCategory() == LlvmTypeAny::Category::Float
			   ? dynamic_cast<LlvmTypeFloat*>(type)
			   : nullptr;
	}
	
	LlvmTypePointer* LlvmTypeManager::castToPointerType(LlvmTypeAny* type) const
	{
		return type->getCategory() == LlvmTypeAny::Category::Pointer
			   ? dynamic_cast<LlvmTypePointer*>(type)
			   : nullptr;
	}
	
	LlvmTypeArray* LlvmTypeManager::castToArrayType(LlvmTypeAny* type) const
	{
		return type->getCategory() == LlvmTypeAny::Category::Array
			   ? dynamic_cast<LlvmTypeArray*>(type)
			   : nullptr;
	}
	
	LlvmTypeFunction* LlvmTypeManager::castToFunctionType(LlvmTypeAny* type) const
	{
		return type->getCategory() == LlvmTypeAny::Category::Function
			   ? dynamic_cast<LlvmTypeFunction*>(type)
			   : nullptr;
	}
	
	LlvmTypeStruct* LlvmTypeManager::castToStructType(LlvmTypeAny* type) const
	{
		return type->getCategory() == LlvmTypeAny::Category::Struct
			   ? dynamic_cast<LlvmTypeStruct*>(type)
			   : nullptr;
	}
}
