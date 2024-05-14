#include "./x-module-cstd.h"

namespace eokas {
    omis_module_cstd_t::omis_module_cstd_t(omis_context_t* context)
            : omis_module_t(context, "c") {}

    bool omis_module_cstd_t::main() {
        this->printf();
        this->sprintf();
        this->malloc();
        this->free();
        this->strlen();
        return true;
    }

    void omis_module_cstd_t::printf() {
        String name = "printf";
        omis_type_t* ret = this->type_i32();
        std::vector<omis_type_t*> args = {this->type_bytes()};
        bool varg = true;

        auto func = this->value_func(name, ret, args, varg);
        this->add_value_symbol(name, func);
    }

    void omis_module_cstd_t::sprintf() {
        String name = "sprintf";
        omis_type_t* ret = type_i32();
        std::vector<omis_type_t*> args = {type_bytes(), type_bytes()};
        bool varg = true;

        auto func = this->value_func(name, ret, args, varg);
        this->add_value_symbol(name, func);
    }

    void omis_module_cstd_t::malloc() {
        String name = "malloc";
        omis_type_t* ret = type_bytes();
        std::vector<omis_type_t*> args = {type_i64()};
        bool varg = false;

        auto func = this->value_func(name, ret, args, varg);
        this->add_value_symbol(name, func);
    }

    void omis_module_cstd_t::free() {
        String name = "free";
        omis_type_t* ret = type_void();
        std::vector<omis_type_t*> args = {type_bytes()};
        bool varg = false;

        auto func = this->value_func(name, ret, args, varg);
        this->add_value_symbol(name, func);
    }

    void omis_module_cstd_t::strlen() {
        String name = "strlen";
        omis_type_t* ret = type_i32();
        std::vector<omis_type_t*> args = {type_bytes()};
        bool varg = true;

        auto func = this->value_func(name, ret, args, varg);
        this->add_value_symbol(name, func);
    }
}
