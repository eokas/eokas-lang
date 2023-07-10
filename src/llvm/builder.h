
#ifndef _EOKAS_LLVM_BUILDER_H_
#define _EOKAS_LLVM_BUILDER_H_

#include "./header.h"
#include "./scope.h"

namespace eokas
{
	struct llvm_basic_builder_t
	{
		llvm::LLVMContext& context;
		
		llvm::Type* type_void;
		llvm::Type* type_i8;
		llvm::Type* type_i16;
		llvm::Type* type_i32;
		llvm::Type* type_i64;
		llvm::Type* type_u8;
		llvm::Type* type_u16;
		llvm::Type* type_u32;
		llvm::Type* type_u64;
		llvm::Type* type_f32;
		llvm::Type* type_f64;
		llvm::Type* type_bool;
		llvm::Type* type_cstr;
		llvm::Type* type_void_ptr;
		
		llvm_basic_builder_t(llvm::LLVMContext& context);
		virtual ~llvm_basic_builder_t();
		
		virtual void resolve() = 0;
		
		String get_type_name(llvm::Type* type);
		llvm::Value* get_default_value(llvm::Type* type);
	};
	
	struct llvm_module_builder_t : llvm_basic_builder_t
	{
		llvm::Module module;
		llvm::IRBuilder<> builder;
		std::map<String, struct llvm_type_builder_t*> types;
		std::map<String, struct llvm_func_builder_t*> funcs;
		
		llvm_module_builder_t(llvm::LLVMContext& context, const String& name);
		virtual ~llvm_module_builder_t();
		
		virtual void resolve() override;
		
		llvm_type_builder_t* add_type(const String& name, llvm_type_builder_t* type);
		llvm_type_builder_t* get_type(const String& name);
		
		llvm_func_builder_t* add_func(const String& name, llvm::Type* ret, const std::vector<llvm::Type*>& args, bool varg);
		llvm_func_builder_t* get_func(const String& name);
	};
	
	struct llvm_type_builder_t : llvm_basic_builder_t
	{
		struct member_t
		{
			String name;
			llvm::Type* type;
			llvm::Value* value;
		};
		
		llvm_module_builder_t& module;
		String name;
		llvm::StructType* handle;
		std::vector<member_t> members;
		
		llvm_type_builder_t(llvm_module_builder_t& module, const String& name);;
		
		virtual void resolve() override;
		
		bool extends(const String& base);
		bool extends(llvm_type_builder_t* base);
		
		member_t* add_member(const String& name, llvm::Type* type, llvm::Value* value = nullptr);
		member_t* add_member(const member_t* other);
		member_t* get_member(const String& name);
		member_t* get_member(size_t index);
		size_t get_member_index(const String& name) const;
		bool is_final_member(const String& name) const;
	};
	
	struct llvm_func_builder_t : llvm_basic_builder_t
	{
		llvm_module_builder_t* module;
		llvm_type_builder_t* owner;
		
		llvm::FunctionType* type;
		llvm::Function* handle;
		
		llvm::IRBuilder<> IR;
		
		llvm_func_builder_t(
			llvm_module_builder_t* module,
			llvm_type_builder_t* owner,
			const String& name,
			llvm::Type* retT,
			const std::vector<llvm::Type*> argsT,
			bool varg);
		
		virtual void resolve() override;
		
		/**
		 * For ref-types: transform multi-level pointer to one-level pointer.
		 * For val-types: transform multi-level pointer to real value.
		 * */
		llvm::Value* get_value(llvm::Value* value);
		/**
		 * Transform the multi-level pointer value to one-level pointer type value.
		 * Ignores literal value.
		 * */
		llvm::Value* ref_value(llvm::Value* value);
		
		llvm::BasicBlock* add_basic_block(const String& name);
		void add_tail_ret();
		
		bool is_array_type(llvm::Type* type);
		
		llvm::Value* make(llvm::Type* type);
		llvm::Value* make(llvm::Type* type, llvm::Value* count);
		llvm::Value* make(llvm_type_builder_t* type);
		void free(llvm::Value* ptr);
		
		llvm::Value* array_set(llvm::Value* array, const llvm::ArrayRef<llvm::Value*>& elements);
		llvm::Value* array_get(llvm::Value* array, llvm::Value* index);
		
		llvm::Value* cstr_len(llvm::Value* val);
		llvm::Value* cstr_from_value(llvm::Value* val);
		llvm::Value* cstr_from_string(llvm::Value* str);
		llvm::Value* cstr_from_number(llvm::Value* val);
		llvm::Value* cstr_from_bool(llvm::Value* val);
		
		llvm::Value* string_make(const char* cstr);
		llvm::Value* string_from_cstr(llvm::Value* cstr);
		llvm::Value* string_from_value(llvm::Value* val);
		llvm::Value* string_get_char(llvm::Value* str, llvm::Value* index);
		llvm::Value* string_concat(llvm::Value* str1, llvm::Value* str2);
		
		llvm::Value* print(const std::vector<llvm::Value*>& args);
	};
}

#endif //_EOKAS_LLVM_BUILDER_H_
