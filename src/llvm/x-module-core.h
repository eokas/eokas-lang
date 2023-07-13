
#ifndef _EOKAS_LLVM_MODULE_CORE_H_
#define _EOKAS_LLVM_MODULE_CORE_H_

#include "./models.h"

#include <llvm/IR/IRBuilder.h>

namespace eokas
{
	struct llvm_type_object_t : llvm_type_t
	{
		llvm_type_object_t(llvm_module_t* module)
			: llvm_type_t(module, "Object")
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
			llvm::Type* ret = module->get_type("TypeInfo")->handle;
			std::vector<llvm::Type*> args = {};
			
			auto func = new llvm_function_t(module, name, ret, args, false);
			module->add_value(name, func);
			
			return func->func;
		}
		
		llvm::Value* string()
		{
			String name = "string";
			llvm::Type* ret = module->get_type("String")->handle;
			std::vector<llvm::Type*> args = {};
			
			auto func = new llvm_function_t(module, name, ret, args, false);
			module->add_value(name, func);
			
			return func->func;
		}
		
		llvm::Value* hash()
		{
			String name = "hash";
			llvm::Type* ret = module->type_i64;
			std::vector<llvm::Type*> args = {};
			
			auto func = new llvm_function_t(module, name, ret, args, false);
			module->add_value(name, func);
			
			return func->func;
		}
	};
	
	struct llvm_type_typeinfo_t : llvm_type_t
	{
		llvm_type_typeinfo_t(llvm_module_t* module)
			: llvm_type_t(module, "TypeInfo")
		{ }
		
		virtual void body() override
		{
			llvm_type_t::body();
			this->extends("Object");
			this->add_member("name", nullptr, length());
			this->add_member("base", nullptr, toUpper());
			this->add_member("fields", nullptr, toLower());
			this->add_member("methods", nullptr, toLower());
			this->add_member("make", nullptr, toLower());
		}
		
		llvm::Value* length()
		{
			String name = "length";
			llvm::Type* ret = module->type_i64;
			std::vector<llvm::Type*> args = {};
			
			auto func = new llvm_function_t(module, name, ret, args, false);
			module->add_value(name, func);
			
			return func->func;
		}
		
		llvm::Value* toUpper()
		{
			String name = "toUpper";
			llvm::Type* ret = module->get_type("string")->handle;
			std::vector<llvm::Type*> args = {};
			
			auto func = new llvm_function_t(module, name, ret, args, false);
			module->add_value(name, func);
			
			return func->func;
		}
		
		llvm::Value* toLower()
		{
			String name = "toLower";
			llvm::Type* ret = module->get_type("string")->handle;
			std::vector<llvm::Type*> args = {};
			
			auto func = new llvm_function_t(module, name, ret, args, false);
			module->add_value(name, func);
			
			return func->func;
		}
	};
	
	struct llvm_type_string_t : llvm_type_t
	{
		llvm_type_string_t(llvm_module_t* module)
			: llvm_type_t(module, "String")
		{ }
		
		virtual void body() override
		{
			llvm_type_t::body();
			this->extends("Object");
			this->add_member("data", module->type_cstr);
			this->add_member("len", module->type_u64);
			this->add_member("length", nullptr, length());
			this->add_member("toUpper", nullptr, toUpper());
			this->add_member("toLower", nullptr, toLower());
		}
		
		llvm::Value* length()
		{
			String name = "length";
			llvm::Type* ret = module->type_i64;
			std::vector<llvm::Type*> args = {};
			
			auto func = new llvm_function_t(module, name, ret, args, false);
			module->add_value(name, func);
			
			return func->func;
		}
		
		llvm::Value* toUpper()
		{
			String name = "toUpper";
			llvm::Type* ret = module->get_type("string")->handle;
			std::vector<llvm::Type*> args = {};
			
			auto func = new llvm_function_t(module, name, ret, args, false);
			module->add_value(name, func);
			
			return func->func;
		}
		
		llvm::Value* toLower()
		{
			String name = "toLower";
			llvm::Type* ret = module->get_type("string")->handle;
			std::vector<llvm::Type*> args = {};
			
			auto func = new llvm_function_t(module, name, ret, args, false);
			module->add_value(name, func);
			
			return func->func;
		}
	};
	
	struct llvm_type_array_t : llvm_type_t
	{
		llvm_type_t* element_type;
		
		llvm_type_array_t(llvm_module_t* module, llvm_type_t* element_type)
			: llvm_type_t(module, String::format("Array<%s>", element_type->name.cstr()))
			, element_type(element_type)
		{ }
		
		virtual void body() override
		{
			llvm_type_t::body();
			
			auto eleT = this->element_type->handle;
			auto dataT = llvm::ArrayType::get(eleT, 0)->getPointerTo();
			
			this->extends("Object");
			this->add_member("data", dataT);
			this->add_member("len", module->type_u64);
		}
	};
	
	struct llvm_module_core_t : llvm_module_t
	{
		llvm_module_core_t(llvm::LLVMContext& context)
			: llvm_module_t(context, "core")
		{ }
		
		void body() override
		{
			this->add_type("Object", new llvm_type_object_t(this));
			this->add_type("TypeInfo", new llvm_type_typeinfo_t(this));
			this->add_type("String", new llvm_type_string_t(this));
			
			this->define_func_print();
		}
		
		llvm::Function* define_func_print()
		{
			String name = "print";
			llvm::Type* ret = type_i32;
			
			llvm::Type* type_string_ptr = this->get_type("String")->handle;
			std::vector<llvm::Type*> args = {type_string_ptr};

			auto func = new llvm_function_t(this, name, ret, args, false);
			this->add_value(name, func);
			
			auto& IR = func->IR;
			{
				auto* entry = func->add_basic_block("entry");
				IR.SetInsertPoint(entry);
				llvm::Value* arg0 = func->func->getArg(0);
				llvm::Value* cstr = func->cstr_from_string(arg0);
				llvm::Value* retval = func->print({cstr});
				IR.CreateRet(retval);
			}
			return func->func;
		}
	};
}

#endif //_EOKAS_LLVM_MODULE_CORE_H_
