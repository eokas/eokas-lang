
#include "./x-module-core.h"

namespace eokas {
    omis_type_object_t::omis_type_object_t(omis_module_t* module)
            : omis_struct_t(module, nullptr) {
    }

    bool omis_type_object_t::main() {
        this->add_member("typeinfo", typeinfo());
        this->add_member("string", string());
        this->add_member("hash", hash());
        return true;
    }

    omis_value_t* omis_type_object_t::typeinfo() {
        String name = "typeinfo";
        omis_type_t* ret = module->get_type_symbol("TypeInfo", true)->type;
        std::vector<omis_type_t*> args = {};

        auto func = module->value_func(name, ret, args, false);
        return func;
    }

    omis_value_t* omis_type_object_t::string() {
        String name = "string";
        omis_type_t* ret = module->get_type_symbol("String", true)->type;
        std::vector<omis_type_t*> args = {};

        auto func = module->value_func(name, ret, args, false);
        return func;
    }

    omis_value_t* omis_type_object_t::hash() {
        String name = "hash";
        omis_type_t* ret = module->type_i64();
        std::vector<omis_type_t*> args = {};

        auto func = module->value_func(name, ret, args, false);
        return func;
    }
}

namespace eokas {
    omis_type_typeinfo_t::omis_type_typeinfo_t(omis_module_t* module)
            : omis_type_object_t(module) {}

    bool omis_type_typeinfo_t::main() {
        if (!omis_type_object_t::main())
            return false;
        this->extends("Object");
        this->add_member("name", name());
        this->add_member("base", base());
        this->add_member("fields", fields());
        this->add_member("methods", methods());
        this->add_member("make", make());
        return true;
    }

    omis_value_t* omis_type_typeinfo_t::name() {
        String name = "name";
        omis_type_t* ret = module->type_i64();
        std::vector<omis_type_t*> args = {};

        auto func = module->value_func(name, ret, args, false);
        return func;
    }

    omis_value_t* omis_type_typeinfo_t::base() {
        String name = "base";
        omis_type_t* ret = module->get_type_symbol("string", true)->type;
        std::vector<omis_type_t*> args = {};

        auto func = module->value_func(name, ret, args, false);
        return func;
    }

    omis_value_t* omis_type_typeinfo_t::fields() {
        String name = "fields";
        omis_type_t* ret = module->get_type_symbol("string", true)->type;
        std::vector<omis_type_t*> args = {};

        auto func = module->value_func(name, ret, args, false);
        return func;
    }

    omis_value_t* omis_type_typeinfo_t::methods() {
        String name = "methods";
        omis_type_t* ret = module->get_type_symbol("string", true)->type;
        std::vector<omis_type_t*> args = {};

        auto func = module->value_func(name, ret, args, false);
        return func;
    }

    omis_value_t* omis_type_typeinfo_t::make() {
        String name = "make";
        omis_type_t* ret = module->get_type_symbol("string", true)->type;
        std::vector<omis_type_t*> args = {};

        auto func = module->value_func(name, ret, args, false);
        return func;
    }
}

namespace eokas {
    omis_type_string_t::omis_type_string_t(omis_module_t* module)
            : omis_type_object_t(module) {}

    bool omis_type_string_t::main() {
        if (!omis_type_object_t::main())
            return false;
        this->extends("Object");
        this->add_member("data", module->get_type_symbol("$cstr", true)->type);
        this->add_member("len", module->get_type_symbol("u64", true)->type);
        this->add_member("length", length());
        this->add_member("toUpper", toUpper());
        this->add_member("toLower", toLower());
        return true;
    }

    omis_value_t* omis_type_string_t::length() {
        String name = "length";
        omis_type_t* ret = module->type_i64();
        std::vector<omis_type_t*> args = {};

        auto func = module->value_func(name, ret, args, false);
        return func;
    }

    omis_value_t* omis_type_string_t::toUpper() {
        String name = "toUpper";
        omis_type_t* ret = module->get_type_symbol("string", true)->type;
        std::vector<omis_type_t*> args = {};

        auto func = module->value_func(name, ret, args, false);

        return func;
    }

    omis_value_t* omis_type_string_t::toLower() {
        String name = "toLower";
        omis_type_t* ret = module->get_type_symbol("string", true)->type;
        std::vector<omis_type_t*> args = {};

        auto func = module->value_func(name, ret, args, false);

        return func;
    }
}

namespace eokas {
    omis_module_core_t::omis_module_core_t(omis_bridge_t* bridge)
            : omis_module_t("core", bridge) {}

    bool omis_module_core_t::main() {
        if (!omis_module_t::main())
            return false;
        /*
        this->add_type_symbol("Object", this->create_type<omis_type_object_t>());
        this->add_type_symbol("TypeInfo", this->create_type<omis_type_typeinfo_t>());
        this->add_type_symbol("i8", this->create_type<omis_type_i8_t>());
        this->add_type_symbol("i16", this->create_type<omis_type_i16_t>());
        this->add_type_symbol("i32", this->create_type<omis_type_i32_t>());
        this->add_type_symbol("i64", this->create_type<omis_type_i64_t>());
        this->add_type_symbol("u8", this->create_type<omis_type_u8_t>());
        this->add_type_symbol("u16", this->create_type<omis_type_u16_t>());
        this->add_type_symbol("u32", this->create_type<omis_type_u32_t>());
        this->add_type_symbol("u64", this->create_type<omis_type_u64_t>());
        this->add_type_symbol("f32", this->create_type<omis_type_f32_t>());
        this->add_type_symbol("f64", this->create_type<omis_type_f64_t>());
        this->add_type_symbol("bool", this->create_type<omis_type_bool_t>());
        this->add_type_symbol("String", this->create_type<omis_type_string_t>());

        this->add_value_symbol("print", this->define_func_print());
         */

        return true;
    }

    omis_func_t* omis_module_core_t::define_func_print() {
        String name = "print";
        omis_type_t* ret = type_i32();

        omis_type_t* type_string_ptr = this->get_type_symbol("String", true)->type;
        std::vector<omis_type_t*> args = {type_string_ptr};

        auto func = this->value_func(name, ret, args, false);
        {
            /*
            auto* entry = func->create_block("entry");
            func->activate_block(entry)
            llvm::Value* arg0 = func->handle->getArg(0);
            llvm::Value* cstr = func->cstr_from_string(arg0);
            llvm::Value* retval = func->print({cstr});
            IR.CreateRet(retval);
            */
        }

        return func;
    }
}
