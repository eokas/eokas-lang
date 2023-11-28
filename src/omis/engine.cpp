
#include "./engine.h"
#include "./model.h"

namespace eokas {
    omis_engine_t::omis_engine_t()
        : bridge(nullptr)
        , modules() { }

    omis_engine_t::~omis_engine_t() {
        this->bridge = nullptr;
        _DeleteMap(this->modules);
    }

    void omis_engine_t::set_bridge(omis_bridge_t *bridge) {
        this->bridge = bridge;
    }

    omis_bridge_t* omis_engine_t::get_bridge() {
        return this->bridge;
    }

    omis_module_t* omis_engine_t::load_ast(eokas::ast_node_module_t *node) {
        return nullptr;
    }

    void omis_engine_t::jit(eokas::omis_module_t *mod) {

    }

    void omis_engine_t::aot(eokas::omis_module_t *mod) {

    }
}
