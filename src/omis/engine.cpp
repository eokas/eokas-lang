
#include "./engine.h"
#include "./model.h"
#include "./bridge.h"

namespace eokas {
    omis_engine_t::omis_engine_t(omis_bridge_t* bridge)
        : bridge(bridge)
        , modules() { }

    omis_engine_t::~omis_engine_t() {
        _DeleteMap(this->modules);
        this->bridge = nullptr;
    }

    omis_bridge_t* omis_engine_t::get_bridge() {
        return this->bridge;
    }

    bool omis_engine_t::add_module(const String &name, omis_module_t *mod) {
        auto iter = this->modules.find(name);
        if(iter != this->modules.end())
            return false;
        this->modules.insert(std::make_pair(name, mod));
        return true;
    }

    omis_module_t* omis_engine_t::get_module(const String& name) {
        auto iter = this->modules.find(name);
        if(iter == this->modules.end())
            return nullptr;
        return iter->second;
    }

    omis_module_t* omis_engine_t::load_module(const String& name, const omis_loading_t& loading) {
        if(this->get_module(name) != nullptr)
            return nullptr;
        auto mod = loading();
        if(mod == nullptr)
            return nullptr;
        if(!this->add_module(name, mod))
            return nullptr;
        return mod;
    }

    bool omis_engine_t::jit(eokas::omis_module_t *mod) {
        return bridge->jit(mod->get_handle());
    }

    bool omis_engine_t::aot(eokas::omis_module_t *mod) {
        return bridge->aot(mod->get_handle());
    }
}
