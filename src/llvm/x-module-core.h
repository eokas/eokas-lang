
#ifndef _EOKAS_LLVM_MODULE_CORE_H_
#define _EOKAS_LLVM_MODULE_CORE_H_

#include <llvm/IR/IRBuilder.h>

#include "./header.h"
#include "./builder.h"

namespace eokas
{
	struct llvm_type_object_t : llvm_type_builder_t
	{
		llvm_type_object_t(llvm_module_builder_t& module)
			: llvm_type_builder_t(module, "Object")
		{ }
		
		void body() override
		{
			this->add_member("typeinfo", nullptr, typeinfo());
			this->add_member("string", nullptr, string());
			this->add_member("hash", nullptr, hash());
		}
		
		llvm::Value* typeinfo()
		{
			String name = "typeinfo";
			llvm::Type* ret = module.get_type("TypeInfo")->handle;
			std::vector<llvm::Type*> args = {};
			auto func = this->module.add_func(name, ret, args, false);
			
			return func->handle;
		}
		
		llvm::Value* string()
		{
			String name = "string";
			llvm::Type* ret = module.get_type("String")->handle;
			std::vector<llvm::Type*> args = {};
			auto func = this->module.add_func(name, ret, args, false);
			
			return func->handle;
		}
		
		llvm::Value* hash()
		{
			String name = "hash";
			llvm::Type* ret = module.type_i64;
			std::vector<llvm::Type*> args = {};
			auto func = this->module.add_func(name, ret, args, false);
			
			return func->handle;
		}
	};
	
	struct llvm_type_typeinfo_t : llvm_type_builder_t
	{
		llvm_type_typeinfo_t(llvm_module_builder_t& module)
			: llvm_type_builder_t(module, "TypeInfo")
		{ }
		
		virtual void resolve() override
		{
			this->extends("Object");
			this->add_member("name", nullptr, length());
			this->add_member("base", nullptr, toUpper());
			this->add_member("fields", nullptr, toLower());
			this->add_member("methods", nullptr, toLower());
			this->add_member("make", nullptr, toLower());
			llvm_type_builder_t::resolve();
		}
		
		llvm::Value* length()
		{
			String name = "length";
			llvm::Type* ret = module.type_i64;
			std::vector<llvm::Type*> args = {};
			auto func = this->module.add_func(name, ret, args, false);
			
			return func->handle;
		}
		
		llvm::Value* toUpper()
		{
			String name = "toUpper";
			llvm::Type* ret = module.get_type("string")->handle;
			std::vector<llvm::Type*> args = {};
			auto func = this->module.add_func(name, ret, args, false);
			
			return func->handle;
		}
		
		llvm::Value* toLower()
		{
			String name = "toLower";
			llvm::Type* ret = module.get_type("string")->handle;
			std::vector<llvm::Type*> args = {};
			auto func = this->module.add_func(name, ret, args, false);
			
			return func->handle;
		}
	};
	
	struct llvm_type_string_t : llvm_type_builder_t
	{
		llvm_type_string_t(llvm_module_builder_t& module)
			: llvm_type_builder_t(module, "String")
		{ }
		
		virtual void resolve() override
		{
			this->extends("Object");
			this->add_member("data", type_cstr);
			this->add_member("len", type_u64);
			this->add_member("length", nullptr, length());
			this->add_member("toUpper", nullptr, toUpper());
			this->add_member("toLower", nullptr, toLower());
			llvm_type_builder_t::resolve();
		}
		
		llvm::Value* length()
		{
			String name = "length";
			llvm::Type* ret = module.type_i64;
			std::vector<llvm::Type*> args = {};
			auto func = this->module.add_func(name, ret, args, false);
			
			return func->handle;
		}
		
		llvm::Value* toUpper()
		{
			String name = "toUpper";
			llvm::Type* ret = module.get_type("string")->handle;
			std::vector<llvm::Type*> args = {};
			auto func = this->module.add_func(name, ret, args, false);
			
			return func->handle;
		}
		
		llvm::Value* toLower()
		{
			String name = "toLower";
			llvm::Type* ret = module.get_type("string")->handle;
			std::vector<llvm::Type*> args = {};
			auto func = this->module.add_func(name, ret, args, false);
			
			return func->handle;
		}
	};
	
	struct llvm_type_array_t : llvm_type_builder_t
	{
		llvm_type_builder_t* element_type;
		
		llvm_type_array_t(llvm_module_builder_t& module, llvm_type_builder_t* element_type)
			: llvm_type_builder_t(module, String::format("Array<%s>", element_type->name.cstr()))
			, element_type(element_type)
		{ }
		
		virtual void resolve() override
		{
			this->element_type->resolve();
			
			auto eleT = this->element_type->handle;
			auto dataT = llvm::ArrayType::get(eleT, 0)->getPointerTo();
			
			this->extends("Object");
			this->add_member("data", dataT);
			this->add_member("len", type_u64);
			
			llvm_type_builder_t::resolve();
		}
	};
	
	struct llvm_module_core_t : llvm_module_builder_t
	{
		llvm_module_core_t(llvm::LLVMContext& context)
			: llvm_module_builder_t(context, "core")
		{ }
		
		void body() override
		{
			llvm_module_builder_t::body();
			this->add_type("Object", new llvm_type_object_t(*this));
			this->add_type("TypeInfo", new llvm_type_typeinfo_t(*this));
			this->add_type("String", new llvm_type_string_t(*this));
			this->define_func_print();
		}
		
		llvm::Function* define_func_print()
		{
			String name = "print";
			llvm::Type* ret = type_i32;
			std::vector<llvm::Type*> args = {type_string_ptr};
			bool varg = false;

			auto func = this->add_func(name, ret, args, varg);
			auto& IR = func->IR;
			{
				auto* entry = func->add_basic_block("entry");
				IR.SetInsertPoint(entry);
				llvm::Value* arg0 = func->handle->getArg(0);
				llvm::Value* cstr = func->cstr_from_string(arg0);
				llvm::Value* retval = func->print({cstr});
				IR.CreateRet(retval);
			}
			return func->handle;
		}
	};
}

#endif //_EOKAS_LLVM_MODULE_CORE_H_
