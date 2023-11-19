
#ifndef _EOKAS_LLVM_MODULE_CORE_H_
#define _EOKAS_LLVM_MODULE_CORE_H_

#include "./models.h"

#include <llvm/IR/IRBuilder.h>

namespace eokas {
    struct llvm_type_object_t : llvm_type_struct_t {
        llvm_type_object_t(llvm_module_t* module)
                : llvm_type_struct_t(module) {}

        void body() override {
            this->add_member("typeinfo", typeinfo());
            this->add_member("string", string());
            this->add_member("hash", hash());
        }

        llvm_value_t* typeinfo() {
            String name = "typeinfo";
            llvm::Type* ret = module->get_type_symbol("TypeInfo")->type->handle;
            std::vector<llvm::Type*> args = {};

            auto func = module->create_function(name, ret, args, false);
            return func;
        }

        llvm_value_t* string() {
            String name = "string";
            llvm::Type* ret = module->get_type_symbol("String")->type->handle;
            std::vector<llvm::Type*> args = {};

            auto func = module->create_function(name, ret, args, false);
            return func;
        }

        llvm_value_t* hash() {
            String name = "hash";
            llvm::Type* ret = module->type_i64;
            std::vector<llvm::Type*> args = {};

            auto func = module->create_function(name, ret, args, false);
            return func;
        }
    };

    struct llvm_type_typeinfo_t : llvm_type_object_t {
        llvm_type_typeinfo_t(llvm_module_t* module)
                : llvm_type_object_t(module) {}

        virtual void body() override {
            llvm_type_object_t::body();
            this->extends("Object");
            this->add_member("name", name());
            this->add_member("base", base());
            this->add_member("fields", fields());
            this->add_member("methods", methods());
            this->add_member("make", make());
        }

        llvm_value_t* name() {
            String name = "name";
            llvm::Type* ret = module->type_i64;
            std::vector<llvm::Type*> args = {};

            auto func = module->create_function(name, ret, args, false);
            return func;
        }

        llvm_value_t* base() {
            String name = "base";
            llvm::Type* ret = module->get_type_symbol("string")->type->handle;
            std::vector<llvm::Type*> args = {};

            auto func = module->create_function(name, ret, args, false);
            return func;
        }

        llvm_value_t* fields() {
            String name = "fields";
            llvm::Type* ret = module->get_type_symbol("string")->type->handle;
            std::vector<llvm::Type*> args = {};

            auto func = module->create_function(name, ret, args, false);
            return func;
        }

        llvm_value_t* methods() {
            String name = "methods";
            llvm::Type* ret = module->get_type_symbol("string")->type->handle;
            std::vector<llvm::Type*> args = {};

            auto func = module->create_function(name, ret, args, false);
            return func;
        }

        llvm_value_t* make() {
            String name = "make";
            llvm::Type* ret = module->get_type_symbol("string")->type->handle;
            std::vector<llvm::Type*> args = {};

            auto func = module->create_function(name, ret, args, false);
            return func;
        }
    };

    struct llvm_type_i8_t : llvm_type_object_t {
        llvm_type_i8_t(llvm_module_t* module)
                : llvm_type_object_t(module) {}
    };

    struct llvm_type_i16_t : llvm_type_object_t {
        llvm_type_i16_t(llvm_module_t* module)
                : llvm_type_object_t(module) {}
    };

    struct llvm_type_i32_t : llvm_type_object_t {
        llvm_type_i32_t(llvm_module_t* module)
                : llvm_type_object_t(module) {}
    };

    struct llvm_type_i64_t : llvm_type_object_t {
        llvm_type_i64_t(llvm_module_t* module)
                : llvm_type_object_t(module) {}
    };

    struct llvm_type_u8_t : llvm_type_object_t {
        llvm_type_u8_t(llvm_module_t* module)
                : llvm_type_object_t(module) {}
    };

    struct llvm_type_u16_t : llvm_type_object_t {
        llvm_type_u16_t(llvm_module_t* module)
                : llvm_type_object_t(module) {}
    };

    struct llvm_type_u32_t : llvm_type_object_t {
        llvm_type_u32_t(llvm_module_t* module)
                : llvm_type_object_t(module) {}
    };

    struct llvm_type_u64_t : llvm_type_object_t {
        llvm_type_u64_t(llvm_module_t* module)
                : llvm_type_object_t(module) {}
    };

    struct llvm_type_f32_t : llvm_type_object_t {
        llvm_type_f32_t(llvm_module_t* module)
                : llvm_type_object_t(module) {}
    };

    struct llvm_type_f64_t : llvm_type_object_t {
        llvm_type_f64_t(llvm_module_t* module)
                : llvm_type_object_t(module) {}
    };

    struct llvm_type_bool_t : llvm_type_object_t {
        llvm_type_bool_t(llvm_module_t* module)
                : llvm_type_object_t(module) {}
    };

    struct llvm_type_string_t : llvm_type_object_t {
        llvm_type_string_t(llvm_module_t* module)
                : llvm_type_object_t(module) {}

        virtual void body() override {
            llvm_type_object_t::body();
            this->extends("Object");
            this->add_member("data", module->get_type_symbol("$cstr")->type);
            this->add_member("len", module->get_type_symbol("u64")->type);
            this->add_member("length", length());
            this->add_member("toUpper", toUpper());
            this->add_member("toLower", toLower());
        }

        llvm_value_t* length() {
            String name = "length";
            llvm::Type* ret = module->type_i64;
            std::vector<llvm::Type*> args = {};

            auto func = module->create_function(name, ret, args, false);
            return func;
        }

        llvm_value_t* toUpper() {
            String name = "toUpper";
            llvm::Type* ret = module->get_type_symbol("string")->type->handle;
            std::vector<llvm::Type*> args = {};

            auto func = module->create_function(name, ret, args, false);

            return func;
        }

        llvm_value_t* toLower() {
            String name = "toLower";
            llvm::Type* ret = module->get_type_symbol("string")->type->handle;
            std::vector<llvm::Type*> args = {};

            auto func = module->create_function(name, ret, args, false);

            return func;
        }
    };

    struct llvm_type_array_t : llvm_type_object_t {
        llvm_type_t* element_type = nullptr;

        llvm_type_array_t(llvm_module_t* module)
                : llvm_type_object_t(module) {}

        void set_element_type(llvm_type_t* ele_type) {
            this->element_type = ele_type;
        }

        virtual void body() override {
            llvm_type_object_t::body();

            auto eleT = this->element_type->handle;
            auto dataT = llvm::ArrayType::get(eleT, 0)->getPointerTo();

            this->extends("Object");
            this->add_member("data", module->create_type(dataT));
            this->add_member("len", module->get_type_symbol("u64")->type);
        }
    };

    struct llvm_module_core_t : llvm_module_t {
        llvm_module_core_t(llvm::LLVMContext& context)
                : llvm_module_t(context, "core") {}

        void body() override {
            this->add_type_symbol("Object", this->create_type<llvm_type_object_t>());
            this->add_type_symbol("TypeInfo", this->create_type<llvm_type_typeinfo_t>());
            this->add_type_symbol("i8", this->create_type< llvm_type_i8_t>());
            this->add_type_symbol("i16", this->create_type< llvm_type_i16_t>());
            this->add_type_symbol("i32", this->create_type< llvm_type_i32_t>());
            this->add_type_symbol("i64", this->create_type< llvm_type_i64_t>());
            this->add_type_symbol("u8", this->create_type< llvm_type_u8_t>());
            this->add_type_symbol("u16", this->create_type< llvm_type_u16_t>());
            this->add_type_symbol("u32", this->create_type< llvm_type_u32_t>());
            this->add_type_symbol("u64", this->create_type< llvm_type_u64_t>());
            this->add_type_symbol("f32", this->create_type< llvm_type_f32_t>());
            this->add_type_symbol("f64", this->create_type< llvm_type_f64_t>());
            this->add_type_symbol("bool", this->create_type< llvm_type_bool_t>());
            this->add_type_symbol("String", this->create_type< llvm_type_string_t>());

            this->add_value_symbol("print", this->define_func_print());
        }

        llvm_function_t* define_func_print() {
            String name = "print";
            llvm::Type* ret = type_i32;

            llvm::Type* type_string_ptr = this->get_type_symbol("String")->type->handle;
            std::vector<llvm::Type*> args = {type_string_ptr};

            auto func = this->create_function(name, ret, args, false);

            auto& IR = func->IR;
            {
                auto* entry = func->add_basic_block("entry");
                IR.SetInsertPoint(entry);
                llvm::Value* arg0 = func->handle->getArg(0);
                llvm::Value* cstr = func->cstr_from_string(arg0);
                llvm::Value* retval = func->print({cstr});
                IR.CreateRet(retval);
            }

            return func;
        }
    };
}

#endif //_EOKAS_LLVM_MODULE_CORE_H_
