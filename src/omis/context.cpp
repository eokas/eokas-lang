
#include "./context.h"
#include "./model.h"
#include "./bridge.h"
#include "./llvm/llvm.h"
#include "./x-module-core.h"
#include "./x-module-cstd.h"

namespace eokas {
    omis_context_t::omis_context_t()
        : bridge(nullptr)
        , modules() {
        bridge = llvm_init();
    }

    omis_context_t::~omis_context_t() {
        _DeleteMap(this->modules);
        llvm_quit(this->bridge);
    }

    omis_bridge_t* omis_context_t::get_bridge() {
        return this->bridge;
    }

    omis_module_t* omis_context_t::get_module(const String& name) {
        auto iter = this->modules.find(name);
        if(iter == this->modules.end())
            return nullptr;
        return iter->second;
    }

    bool omis_context_t::add_module(const String &name, omis_module_t *mod) {
        auto iter = this->modules.find(name);
        if(iter != this->modules.end())
            return false;
        this->modules.insert(std::make_pair(name, mod));
        return true;
    }

    omis_module_t* omis_context_t::load_module(const String& name, const omis_lambda_loading_t& loading) {
        if(this->get_module(name) != nullptr)
            return nullptr;
        auto mod = loading();
        if(mod == nullptr)
            return nullptr;
        if(!this->add_module(name, mod))
            return nullptr;
        return mod;
    }

    bool omis_context_t::load_default_modules() {
        #define _load_default_module(type) {\
            auto mod = new type(this); \
            if(!this->add_module(mod->get_name(), mod)) { \
                return false; \
            } \
        }

        _load_default_module(omis_module_core_t);
        _load_default_module(omis_module_cstd_t);

        return true;
    }

    bool omis_context_t::jit(eokas::omis_module_t *mod) {
        return bridge->jit(mod->get_handle());
    }

    bool omis_context_t::aot(eokas::omis_module_t *mod) {
        return bridge->aot(mod->get_handle());
    }
}
