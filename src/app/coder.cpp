
#include "./coder.h"
#include "../omis/bridge.h"
#include "../omis/model.h"
#include "../omis/engine.h"
#include "../llvm/llvm.h"
#include "../omis/x-module-coder.h"

namespace eokas {
    coder_t::coder_t() {
        engine = new omis_engine_t();
    }

    coder_t::~coder_t() {
        _DeletePointer(engine);
    }

    omis_module_t* coder_t::encode(ast_node_module_t* node) {
        return engine->load_module(node->name, [&]() -> omis_module_t* {
            omis_module_coder_t* mod = new omis_module_coder_t(engine->get_bridge(), node->name);
            if(!mod->encode_module(node)) {
                _DeletePointer(mod);
                return nullptr;
            }
            return mod;
        });
    }

    void coder_t::jit(omis_module_t* mod) {
        engine->jit(mod);
    }

    void coder_t::aot(omis_module_t* mod) {
        engine->aot(mod);
    }
}
