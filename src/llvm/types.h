
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
            Integer, Float, Pointer, Array, Function, Struct,
        };

        LlvmTypeAny(Category category, llvm::Type* type);
        virtual ~LlvmTypeAny();

        bool operator==(const LlvmTypeAny& other);
        bool operator!=(const LlvmTypeAny& other);

        Category getCategory();
        llvm::Type* getHandle();
        llvm::Value* getDefault();

	protected:
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
        LlvmTypePointer(llvm::Type* type);

        llvm::Type* getElementType();
    };

    class LlvmTypeArray :public LlvmTypeAny
    {
    public:
        LlvmTypeArray(llvm::Type* type);

        llvm::Type* getElementType();
    };

    class LlvmTypeFunction :public LlvmTypeAny
    {
    public:
        LlvmTypeFunction(llvm::Type* type);

    	
    };

    class LlvmTypeStruct :public LlvmTypeAny
    {
	public:
		struct Member
		{
			String name;
			LlvmTypeAny* type;
			llvm::Value* value;
		};
		
		LlvmTypeStruct(llvm::Type* type);
		virtual ~LlvmTypeStruct();
		
		Member* addMember(const String& name, LlvmTypeAny* type, llvm::Value* value);
		Member* addMember(const Member* other);
		Member* getMember(const String& name);
		Member* getMember(size_t index);
		size_t getMemberIndex(const String& name) const;
		
		void resolve();
		bool isResolved() const;
		
	private:
		std::vector<Member*> mMembers;
		bool mResolved;
    };
	
	class LlvmTypeManager
	{
	public:
		llvm::Type* ty_void;
		llvm::Type* ty_i8;
		llvm::Type* ty_i16;
		llvm::Type* ty_i32;
		llvm::Type* ty_i64;
		llvm::Type* ty_u8;
		llvm::Type* ty_u16;
		llvm::Type* ty_u32;
		llvm::Type* ty_u64;
		llvm::Type* ty_f32;
		llvm::Type* ty_f64;
		llvm::Type* ty_bool;
		llvm::Type* ty_ptr_void;
		llvm::Type* ty_ptr_u8;
		
	public:
		LlvmTypeManager(llvm::LLVMContext& context);
		~LlvmTypeManager();
		
		LlvmTypeInteger* integerType(u32_t bits, bool isUnsigned);
		LlvmTypeFloat* floatType(u32_t bits);
		LlvmTypePointer* pointerType(LlvmTypeAny* elementType);
		LlvmTypeArray* arrayType(LlvmTypeAny* elementType);
		LlvmTypeFunction* functionType(LlvmTypeAny* retType, const std::vector<LlvmTypeAny*>& argsTypes);
		LlvmTypeStruct* structType(const String& name);
		
		LlvmTypeInteger* castToIntegerType(LlvmTypeAny* type) const;
		LlvmTypeFloat* castToFloatType(LlvmTypeAny* type) const;
		LlvmTypePointer* castToPointerType(LlvmTypeAny* type) const;
		LlvmTypeArray* castToArrayType(LlvmTypeAny* type) const;
		LlvmTypeFunction* castToFunctionType(LlvmTypeAny* type) const;
		LlvmTypeStruct* castToStructType(LlvmTypeAny* type) const;
		
	private:
		llvm::LLVMContext& mContext;
		std::vector<LlvmTypeAny*> mTypes;
	};
}

#endif //_EOKAS_LLVM_TYPEDEF_H_
