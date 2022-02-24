#ifndef _EOKAS_LLVM_MODELS_H_
#define _EOKAS_LLVM_MODELS_H_

#include "./header.h"

#include <llvm/IR/IRBuilder.h>

_BeginNamespace(eokas)
	
	struct llvm_scope_t;
	struct llvm_type_t;
	struct llvm_expr_t;
	
	template<typename T>
	struct table_t
	{
		std::map<String, T*> table;
		
		explicit table_t()
			: table()
		{
		}
		
		~table_t()
		{
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
	
	struct llvm_scope_t
	{
		llvm_scope_t* parent;
		llvm::Function* func;
		std::vector<llvm_scope_t*> children;
		
		table_t<llvm_expr_t> symbols;
		table_t<llvm_type_t> types;
		
		explicit llvm_scope_t(llvm_scope_t* parent, llvm::Function* func);
		virtual ~llvm_scope_t();
		
		llvm_scope_t* addChild(llvm::Function* f = nullptr);
		
		bool addSymbol(const String& name, llvm_expr_t* expr);
		llvm_expr_t* getSymbol(const String& name, bool lookup);
		
		bool addType(const String& name, llvm_type_t* type);
		llvm_type_t* getType(const String& name, bool lookup);
	};
	
	struct llvm_type_t
	{
		enum class layout_t
		{
			Sequential, Overlapped,
		};
		
		struct member_t
		{
			String name = "";
			llvm_type_t* type = nullptr;
			llvm_expr_t* value = nullptr;
		};
		
		llvm::LLVMContext& context;
		
		String name;
		layout_t layout;
		std::vector<member_t*> members;
		
		llvm::Type* handle;
		llvm::Value* defval;
		llvm_scope_t* scope;
		
		explicit llvm_type_t(llvm::LLVMContext& context, const String& name, llvm::Type* handle, llvm::Value* defval);
		virtual ~llvm_type_t() noexcept;
		
		void set_layout(layout_t layout);
		member_t* add_member(const String& name, llvm_type_t* type, llvm_expr_t* value);
		member_t* add_member(const member_t* other);
		member_t* get_member(const String& name) const;
		member_t* get_member(size_t index) const;
		size_t get_member_index(const String& name) const;
		
		void resolve(bool force = false);
		
		void ref(llvm_type_t* element_type);
	};
	
	struct llvm_expr_t
	{
		llvm::Value* value = nullptr;
		llvm::Type* type = nullptr;
		struct llvm_scope_t* scope = nullptr;
		
		explicit llvm_expr_t(llvm::Value* value, llvm::Type* type = nullptr);
		virtual ~llvm_expr_t();
		
		bool is_symbol() const;
	};
	
	struct llvm_func_t : llvm_expr_t
	{
		table_t<llvm_expr_t> upvals;
		
		explicit llvm_func_t(llvm::Value* value, llvm::Type* type = nullptr)
			: llvm_expr_t(value, type), upvals()
		{
		}
	};
	
	using llvm_code_delegate_t = std::function<void(llvm::LLVMContext&, llvm::Module& module, llvm::Function* func, llvm::IRBuilder<>& builder)>;
	
	struct llvm_module_t
	{
		llvm::LLVMContext& context;
		llvm::Module module;
		
		llvm_scope_t* root;
		llvm_func_t* entry;
		
		std::vector<llvm_type_t*> types;
		std::vector<llvm_expr_t*> exprs;
		
		std::map<llvm::Type*, llvm_type_t*> typemappings;
		
		llvm_type_t* type_void;
		llvm_type_t* type_i8;
		llvm_type_t* type_i16;
		llvm_type_t* type_i32;
		llvm_type_t* type_i64;
		llvm_type_t* type_u8;
		llvm_type_t* type_u16;
		llvm_type_t* type_u32;
		llvm_type_t* type_u64;
		llvm_type_t* type_f32;
		llvm_type_t* type_f64;
		llvm_type_t* type_bool;
		llvm_type_t* type_i8_ref;
		llvm_type_t* type_string;
		llvm_type_t* type_string_ref;
		
		llvm_module_t(const String& name, llvm::LLVMContext& context);
		~llvm_module_t() noexcept;
		
		llvm_type_t* new_type(const String& name, llvm::Type* handle, llvm::Value* defval);
		llvm_expr_t* new_expr(llvm::Value* value, llvm::Type* type = nullptr);
		void map_type(llvm::Type* handle, llvm_type_t* type);
		llvm_type_t* get_type(llvm::Type* handle);
		
		llvm::Value* get_value(llvm::IRBuilder<>& builder, llvm::Value* value);
		llvm::Value* ref_value(llvm::IRBuilder<>& builder, llvm::Value* value);
		
		llvm::Function* declare_func(const String& name, llvm::Type* ret, const std::vector<llvm::Type*>& args, bool varg);
		llvm::Function* declare_func_puts();
		llvm::Function* declare_func_printf();
		llvm::Function* declare_func_sprintf();
		llvm::Function* declare_func_malloc();
		llvm::Function* declare_func_free();
		
		llvm::Function* define_func(const String& name, llvm::Type* ret, const std::vector<llvm::Type*>& args, bool varg, const llvm_code_delegate_t& body);
		llvm::Function* define_func_print();
		
		llvm::Value* make(llvm::Function* func, llvm::IRBuilder<>& builder, llvm::Type* type);
		void free(llvm::Function* func, llvm::IRBuilder<>& builder, llvm::Value* ptr);
		llvm::Value* make_string(llvm::Function* func, llvm::IRBuilder<>& builder, const char* cstr);
		
		llvm::Value* string_to_cstr(llvm::Function* func, llvm::IRBuilder<>& builder, llvm::Value* val);
		llvm::Value* bool_to_cstr(llvm::Function* func, llvm::IRBuilder<>& builder, llvm::Value* val);
		llvm::Value* number_to_cstr(llvm::Function* func, llvm::IRBuilder<>& builder, llvm::Value* val);
		llvm::Value* value_to_cstr(llvm::Function* func, llvm::IRBuilder<>& builder, llvm::Value* val);
		
		llvm::Value* cstr_to_string(llvm::Function* func, llvm::IRBuilder<>& builder, llvm::Value* val);
		llvm::Value* bool_to_string(llvm::Function* func, llvm::IRBuilder<>& builder, llvm::Value* val);
		llvm::Value* number_to_string(llvm::Function* func, llvm::IRBuilder<>& builder, llvm::Value* val);
		llvm::Value* value_to_string(llvm::Function* func, llvm::IRBuilder<>& builder, llvm::Value* val);
		
		llvm::Value* print(llvm::Function* func, llvm::IRBuilder<>& builder, const std::vector<llvm::Value*>& args);
	};
_EndNamespace(eokas)

#endif//_EOKAS_LLVM_MODELS_H_
