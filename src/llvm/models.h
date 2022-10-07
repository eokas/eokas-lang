#ifndef _EOKAS_LLVM_MODELS_H_
#define _EOKAS_LLVM_MODELS_H_

#include "./header.h"

#include <llvm/IR/IRBuilder.h>

_BeginNamespace(eokas)

	template<typename T, bool gc = true>
	struct table_t
	{
		std::map<String, T*> table;
		
		explicit table_t()
			: table()
		{ }
		
		~table_t()
		{
			if(gc && !this->table.empty())
			{
				_DeleteMap(this->table);
			}
			this->table.clear();
		}
		
		bool add(const String& name, T* object)
		{
			if(this->table.find(name) != this->table.end())
				return false;
			this->table.insert(std::make_pair(name, object));
			return true;
		}
		
		T* get(const String& name)
		{
			auto iter = this->table.find(name);
			if(iter == this->table.end())
				return nullptr;
			return iter->second;
		}
	};

	struct llvm_symbol_t
	{
		struct llvm_scope_t* scope = nullptr;
		llvm::Type* type = nullptr;
		llvm::Value* value = nullptr;
	};
	
	struct llvm_schema_t
	{
		struct llvm_scope_t* scope = nullptr;
		llvm::Type* type = nullptr;
		std::vector<llvm::Type*> generics = {};
		
		llvm::Type* resolve(const std::vector<llvm::Type*>& args)
		{
			for(size_t index = 0; index < this->generics.size(); index++)
			{
				auto gen = llvm::cast<llvm::StructType>(this->generics[index]);
				auto arg = llvm::cast<llvm::StructType>(args[index]);
				llvm_schema_t::fillOpaqueStructType(gen, arg);
			}
			return this->type;
		}
		
		static void fillOpaqueStructType(llvm::StructType* opaqueT, llvm::StructType* structT)
		{
			std::vector<llvm::Type*> body;
			for(uint32_t index = 0; index < structT->getNumElements(); index++)
			{
				auto* elementT = structT->getElementType(index);
				body.push_back(elementT);
			}
			opaqueT->setBody(body);
		}
	};
	
	struct llvm_scope_t
	{
		llvm_scope_t* parent;
		llvm::Function* func;
		std::vector<llvm_scope_t*> children;
		
		table_t<llvm_symbol_t> symbols;
		table_t<llvm_schema_t> schemas;
		
		explicit llvm_scope_t(llvm_scope_t* parent, llvm::Function* func);
		virtual ~llvm_scope_t();
		
		llvm_scope_t* addChild(llvm::Function* f = nullptr);
		
		bool addSymbol(const String& name, llvm::Value* expr);
		bool addSymbol(const String& name, llvm::Type* type);
		llvm_symbol_t* getSymbol(const String& name, bool lookup);
		
		bool addSchema(const String& name, llvm::Type* type);
		llvm_schema_t* getSchema(const String& name, bool lookup);
	};
	
	struct llvm_struct_t
	{
		struct member_t
		{
			String name = "";
			llvm::Type* type = nullptr;
			llvm::Value* value = nullptr;
		};
		
		llvm::LLVMContext& context;
		String name;
		llvm::StructType* type;
		std::vector<member_t*> members;

		explicit llvm_struct_t(llvm::LLVMContext& context, const String& name);
		virtual ~llvm_struct_t() noexcept;
		
		member_t* add_member(const String& name, llvm::Type* type, llvm::Value* value = nullptr);
		member_t* add_member(const member_t* other);
		member_t* get_member(const String& name) const;
		member_t* get_member(size_t index) const;
		size_t get_member_index(const String& name) const;
		
		void resolve();
	};
	
	using llvm_code_delegate_t = std::function<void(llvm::LLVMContext&, llvm::Module& module, llvm::Function* func, llvm::IRBuilder<>& builder)>;
	
	struct llvm_model_t
	{
		static llvm::Function* declare_func(llvm::Module& module, const String& name, llvm::Type* ret, const std::vector<llvm::Type*>& args, bool varg);
		static llvm::Function* define_func(llvm::Module& module, const String& name, llvm::Type* ret, const std::vector<llvm::Type*>& args, bool varg, const llvm_code_delegate_t& body);
		
		/**
		 * For ref-types: transform multi-level pointer to one-level pointer.
		 * For val-types: transform multi-level pointer to real value.
		 * */
		static llvm::Value* get_value(llvm::IRBuilder<>& builder, llvm::Value* value);
		/**
		 * Transform the multi-level pointer value to one-level pointer type value.
		 * Ignores literal value.
		 * */
		static llvm::Value* ref_value(llvm::IRBuilder<>& builder, llvm::Value* value);
	};
	
	struct llvm_module_t
	{
		llvm::LLVMContext& context;
		llvm::Module module;
		
		llvm_scope_t* root;
		llvm::Function* entry;
		
		std::vector<llvm_struct_t*> structs;

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
		llvm::Type* type_string;
		llvm::Type* type_string_ptr;
		llvm::Type* type_void_ptr;
		
		llvm_module_t(const String& name, llvm::LLVMContext& context);
		~llvm_module_t() noexcept;
		
		String get_type_name(llvm::Type* type);
		llvm::Value* get_default_value(llvm::Type* type);
		
		llvm_struct_t* new_struct(const String& name);
		llvm_struct_t* get_struct(const String& name);
		llvm_struct_t* get_struct(llvm::Type* handle);
		
		llvm::Function* declare_func_malloc();
		llvm::Function* declare_func_free();
		llvm::Function* declare_func_printf();
		llvm::Function* declare_func_sprintf();
		llvm::Function* declare_func_strlen();
		
		llvm::Function* define_func_print();
		
		llvm::Type* define_schema_array(llvm::Type* element_type);
		bool is_schema_array(llvm::Type* type);
		
		llvm::Type* define_schema_string();
		
		llvm::Value* make(llvm::Function* func, llvm::IRBuilder<>& builder, llvm::Type* type);
		llvm::Value* make(llvm::Function* func, llvm::IRBuilder<>& builder, llvm::Type* type, llvm::Value* count);
		llvm::Value* make(llvm::Function* func, llvm::IRBuilder<>& builder, llvm_struct_t* type);
		void free(llvm::Function* func, llvm::IRBuilder<>& builder, llvm::Value* ptr);
		
		llvm::Value* array_set(llvm::Function* func, llvm::IRBuilder<>& builder, llvm::Value* array, const llvm::ArrayRef<llvm::Value*>& elements);
		llvm::Value* array_get(llvm::Function* func, llvm::IRBuilder<>& builder, llvm::Value* array, llvm::Value* index);
		
		llvm::Value* cstr_len(llvm::Function* func, llvm::IRBuilder<>& builder, llvm::Value* val);
		llvm::Value* cstr_from_value(llvm::Function* func, llvm::IRBuilder<>& builder, llvm::Value* val);
		llvm::Value* cstr_from_string(llvm::Function* func, llvm::IRBuilder<>& builder, llvm::Value* str);
		llvm::Value* cstr_from_number(llvm::Function* func, llvm::IRBuilder<>& builder, llvm::Value* val);
		llvm::Value* cstr_from_bool(llvm::Function* func, llvm::IRBuilder<>& builder, llvm::Value* val);

		llvm::Value* string_make(llvm::Function* func, llvm::IRBuilder<>& builder, const char* cstr);
		llvm::Value* string_from_cstr(llvm::Function* func, llvm::IRBuilder<>& builder, llvm::Value* cstr);
		llvm::Value* string_from_value(llvm::Function* func, llvm::IRBuilder<>& builder, llvm::Value* val);
		llvm::Value* string_get_char(llvm::Function* func, llvm::IRBuilder<>& builder, llvm::Value* str, llvm::Value* index);
		llvm::Value* string_concat(llvm::Function* func, llvm::IRBuilder<>& builder, llvm::Value* str1, llvm::Value* str2);
		
		llvm::Value* print(llvm::Function* func, llvm::IRBuilder<>& builder, const std::vector<llvm::Value*>& args);
		
	};
_EndNamespace(eokas)

#endif//_EOKAS_LLVM_MODELS_H_
