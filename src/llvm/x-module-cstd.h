
#ifndef _EOKAS_LLVM_MODULE_C_H_
#define _EOKAS_LLVM_MODULE_C_H_

#include "./models.h"

#include <llvm/IR/IRBuilder.h>

namespace eokas
{
	struct llvm_module_cstd_t : llvm_module_t
	{
		llvm_module_cstd_t(llvm::LLVMContext& context)
			: llvm_module_t(context, "c")
		{ }
		
		virtual void body() override
		{
			this->printf();
			this->sprintf();
			this->malloc();
			this->free();
			this->strlen();
		}
		
		void printf()
		{
			String name = "printf";
			llvm::Type* ret = type_i32;
			std::vector<llvm::Type*> args = {type_cstr};
			bool varg = true;
			
			auto func = this->create_function(name, ret, args, varg);
            this->add_value_symbol(name, func);
		}
		
		void sprintf()
		{
			String name = "sprintf";
			llvm::Type* ret = type_i32;
			std::vector<llvm::Type*> args = {type_cstr, type_cstr};
			bool varg = true;
			
			auto func = this->create_function(name, ret, args, varg);
            this->add_value_symbol(name, func);
		}
		
		void malloc()
		{
			String name = "malloc";
			llvm::Type* ret = type_cstr;
			std::vector<llvm::Type*> args = {type_i64};
			bool varg = false;
			
			auto func = this->create_function(name, ret, args, varg);
            this->add_value_symbol(name, func);
		}
		
		void free()
		{
			String name = "free";
			llvm::Type* ret = type_void;
			std::vector<llvm::Type*> args = {type_cstr};
			bool varg = false;
			
			auto func = this->create_function(name, ret, args, varg);
            this->add_value_symbol(name, func);
		}
		
		llvm::Function* strlen()
		{
			String name = "strlen";
			llvm::Type* ret = type_i32;
			std::vector<llvm::Type*> args = {type_cstr};
			bool varg = true;
			
			auto func = this->create_function(name, ret, args, varg);
            this->add_value_symbol(name, func);
		}
	};
}

#endif //_EOKAS_LLVM_MODULE_C_H_
