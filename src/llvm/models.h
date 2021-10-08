
#ifndef _EOKAS_LLVM_MODELS_H_
#define _EOKAS_LLVM_MODELS_H_

#include <libarchaism/archaism.h>
#include <llvm/IR/IRBuilder.h>

namespace llvm
{
	class LLVMContext;
	class Type;
	class Value;
	class Function;
	class Module;
	class BasicBlock;
}

_BeginNamespace(eokas)

	struct llvm_model_t
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
		llvm::Type* type_string;
		llvm::Type* type_i8_ptr;
		llvm::Type* type_string_ptr;

		llvm::Value* const_zero;

		explicit llvm_model_t(llvm::LLVMContext& context);
		
		llvm::Value* get_value(llvm::IRBuilder<>& builder, llvm::Value* value);
		llvm::Value* ref_value(llvm::IRBuilder<>& builder, llvm::Value* value);
		
		llvm::Type* define_type_string();

		llvm::Function* declare_cfunc_puts(llvm::Module* module);
		llvm::Function* declare_cfunc_printf(llvm::Module* module);
		llvm::Function* declare_cfunc_sprintf(llvm::Module* module);
		llvm::Function* declare_cfunc_malloc(llvm::Module* module);
		llvm::Function* declare_cfunc_free(llvm::Module* module);
		
		llvm::Function* define_func_print(llvm::Module* module);
		
		llvm::Value* string_to_cstr(llvm::Module* module, llvm::Function* func, llvm::IRBuilder<>& builder, llvm::Value* val);
		llvm::Value* bool_to_cstr(llvm::Module* module, llvm::Function* func, llvm::IRBuilder<>& builder, llvm::Value* val);
		llvm::Value* number_to_cstr(llvm::Module* module, llvm::Function* func, llvm::IRBuilder<>& builder, llvm::Value* val);
		llvm::Value* value_to_cstr(llvm::Module* module, llvm::Function* func, llvm::IRBuilder<>& builder, llvm::Value* val);
		
		llvm::Value* cstr_to_string(llvm::Module* module, llvm::Function* func, llvm::IRBuilder<>& builder, llvm::Value* val);
		llvm::Value* bool_to_string(llvm::Module* module, llvm::Function* func, llvm::IRBuilder<>& builder, llvm::Value* val);
		llvm::Value* number_to_string(llvm::Module* module, llvm::Function* func, llvm::IRBuilder<>& builder, llvm::Value* val);
		llvm::Value* value_to_string(llvm::Module* module, llvm::Function* func, llvm::IRBuilder<>& builder, llvm::Value* val);
		
		llvm::Value* print(llvm::Module* module, llvm::Function* func, llvm::IRBuilder<>& builder, const std::vector<llvm::Value*>& args);
		
	};
_EndNamespace(eokas)

#endif//_EOKAS_LLVM_MODELS_H_
