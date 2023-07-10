
#ifndef _EOKAS_LLVM_MODULE_C_H_
#define _EOKAS_LLVM_MODULE_C_H_

#include <llvm/IR/IRBuilder.h>

#include "./header.h"
#include "./builder.h"

namespace eokas
{
	struct llvm_module_core_t : llvm_module_builder_t
	{
		llvm_module_core_t(llvm::LLVMContext& context)
			: llvm_module_builder_t(context, "c")
		{ }
		
		virtual void resolve() override
		{
			this->printf();
			this->sprintf();
			this->malloc();
			this->free();
			
			llvm_module_builder_t::resolve();
		}
		
		llvm::Function* printf()
		{
			String name = "printf";
			llvm::Type* ret = type_i32;
			std::vector<llvm::Type*> args = {type_cstr};
			bool varg = true;
			
			auto func = this->add_func(name, ret, args, varg);
			return func->handle;
		}
		
		llvm::Function* sprintf()
		{
			String name = "sprintf";
			llvm::Type* ret = type_i32;
			std::vector<llvm::Type*> args = {type_cstr, type_cstr};
			bool varg = true;
			
			auto func = this->add_func(name, ret, args, varg);
			return func->handle;
		}
		
		llvm::Function* malloc()
		{
			String name = "malloc";
			llvm::Type* ret = type_cstr;
			std::vector<llvm::Type*> args = {type_i64};
			bool varg = false;
			
			auto func = this->add_func(name, ret, args, varg);
			return func->handle;
		}
		
		llvm::Function* free()
		{
			String name = "free";
			llvm::Type* ret = type_void;
			std::vector<llvm::Type*> args = {type_cstr};
			bool varg = false;
			
			auto func = this->add_func(name, ret, args, varg);
			return func->handle;
		}
		
		llvm::Function* strlen()
		{
			String name = "strlen";
			llvm::Type* ret = type_i32;
			std::vector<llvm::Type*> args = {type_cstr};
			bool varg = true;
			
			auto func = this->add_func(name, ret, args, varg);
			return func->handle;
		}
	};
}

#endif //_EOKAS_LLVM_MODULE_C_H_
