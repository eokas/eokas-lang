
#ifndef _EOKAS_LLVM_MODELS_H_
#define _EOKAS_LLVM_MODELS_H_

#include "./header.h"
#include "./scope.h"

#include <llvm/IR/IRBuilder.h>

namespace eokas
{
	struct llvm_module_t
	{
		llvm::LLVMContext& context;
		llvm::Module module;
		llvm_scope_t* scope;
		std::vector<llvm_module_t*> usings;
		
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
		
		// TODO: it must be import from module-core.
		llvm::Type* type_string;
		llvm::Type* type_string_ptr;
		
		llvm_module_t(llvm::LLVMContext& context, const String& name);
		virtual ~llvm_module_t();
		
		virtual void begin();
		virtual void body();
		virtual void end();
		
		void using_module(llvm_module_t* other);
		
		bool add_type(const String& name, struct llvm_type_t* type);
		llvm_type_t* get_type(const String& name);
		
		bool add_value(const String& name, struct llvm_value_t* value);
		llvm_value_t* get_value(const String& name);
		
		String get_type_name(llvm::Type* type);
		llvm::Value* get_default_value(llvm::Type* type);
	};
	
	struct llvm_type_t
	{
		struct member_t
		{
			String name;
			llvm::Type* type;
			llvm::Value* value;
		};
		
		llvm_module_t* module;
		String name;
		llvm::StructType* handle;
		std::vector<llvm::Type*> generics = {};
		std::vector<member_t> members;
		
		llvm_type_t(llvm_module_t* module, const String& name);
		
		virtual void begin();
		virtual void body();
		virtual void end();
		
		bool extends(const String& base);
		bool extends(llvm_type_t* base);
		
		member_t* add_member(const String& name, llvm::Type* type, llvm::Value* value = nullptr);
		member_t* add_member(const member_t* other);
		member_t* get_member(const String& name);
		member_t* get_member(size_t index);
		size_t get_member_index(const String& name) const;
		bool is_final_member(const String& name) const;
		
		void resolve_generic_type(const std::vector<llvm::Type*>& args);
		void resolve_opaque_type(llvm::StructType* opaqueT, llvm::StructType* structT);
		
		bool is_value_type() const;
		bool is_reference_type() const;
	};
	
	struct llvm_value_t
	{
		llvm_module_t* module;
		llvm::Value* value;
		
		llvm_value_t(llvm_module_t* module);
	};
	
	struct llvm_function_t : llvm_value_t
	{
		llvm::FunctionType* type;
		llvm::Function* func;
		
		llvm::IRBuilder<> IR;
		
		llvm_function_t(
			llvm_module_t* module,
			const String& name,
			llvm::Type* retT,
			const std::vector<llvm::Type*> argsT,
			bool varg);
		
		virtual void begin();
		virtual void body();
		virtual void end();
		
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
		llvm::Value* make(llvm_type_t* type);
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

#endif //_EOKAS_LLVM_MODELS_H_
