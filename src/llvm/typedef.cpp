#include "./typedef.h"

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

    LlvmTypePointer::LlvmTypePointer(llvm::Type* elementType)
        : LlvmTypeAny(Category::Pointer, elementType->getPointerTo())
        , mElementType(elementType)
    { }

    llvm::Type* LlvmTypePointer::getElementType()
    {
        return mElementType;
    }

    LlvmTypeArray::LlvmTypeArray(llvm::Type* elementType)
        : LlvmTypeAny(Category::Array, llvm::ArrayType::get(elementType, 0))
        , mElementType(elementType)
    {}

    llvm::Type* LlvmTypeArray::getElementType()
    {
        return mElementType;
    }

    LlvmTypeFunction::LlvmTypeFunction(llvm::Type* retType, const std::vector<llvm::Type*> argsTypes)
        : LlvmTypeAny(Category::Function, llvm::FunctionType::get(retType, argsTypes, false))
    {}


}



























