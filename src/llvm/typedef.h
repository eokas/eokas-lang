
#ifndef _EOKAS_LLVM_TYPEDEF_H_
#define _EOKAS_LLVM_TYPEDEF_H_

#include "./header.h"
#include <llvm/IR/IRBuilder.h>

namespace eokas
{
    class LlvmTypeAny
    {
    public:
        enum class Category
        {
            Integer, Float, Pointer, Array, Function, Schema,
        };

        LlvmTypeAny(Category category, llvm::Type* type);
        virtual ~LlvmTypeAny();

        bool operator==(const LlvmTypeAny& other);
        bool operator!=(const LlvmTypeAny& other);

        Category getCategory();
        llvm::Type* getHandle();
        llvm::Value* getDefault();

    private:
        Category mCategory;
        llvm::Type* mHandle;
        llvm::Value* mDefault;
    };

    class LlvmTypeInteger :public LlvmTypeAny
    {
    public:
        LlvmTypeInteger(llvm::Type* type);

        u32_t getBits();
        void setBits(u32_t bits);
        bool isUnsigned();
        void setUnsigned(bool value);

    private:
        u32_t mBits;
        bool mUnsigned;
    };

    class LlvmTypeFloat :public LlvmTypeAny
    {
    public:
        LlvmTypeFloat(llvm::Type* type);

        u32_t getBits();
        void setBits(u32_t bits);

    private:
        u32_t mBits;
    };

    class LlvmTypePointer :public LlvmTypeAny
    {
    public:
        LlvmTypePointer(llvm::Type* elementType);

        llvm::Type* getElementType();

    private:
        llvm::Type* mElementType;
    };

    class LlvmTypeArray :public LlvmTypeAny
    {
    public:
        LlvmTypeArray(llvm::Type* elementType);

        llvm::Type* getElementType();

    private:
        llvm::Type* mElementType;
    };

    class LlvmTypeFunction :public LlvmTypeAny
    {
    public:
        LlvmTypeFunction(llvm::Type* retType, const std::vector<llvm::Type*> argsTypes);

    private:
        llvm::Type* mRetType;
        std::vector<llvm::Type*> mArgsTypes;
    };

    class LlvmTypeSchema :public LlvmTypeAny
    {

    };
}

#endif //_EOKAS_LLVM_TYPEDEF_H_























