
#include "./coder.h"
#include "../omis/bridge.h"
#include "../omis/model.h"
#include "../omis/engine.h"
#include "../llvm/llvm.h"

namespace eokas {
    coder_t::coder_t() {
        bridge = llvm_init();
        engine = new omis_engine_t(bridge);
    }

    coder_t::~coder_t() {
        _DeletePointer(engine);
        llvm_quit(bridge);
    }

    void coder_t::encode(ast_node_module_t* node) {
        engine->load_module(node->name, [&]() -> omis_module_t* {
            auto mb = engine->get_bridge()->make_module(node->name);
            omis_module_t* mod = new omis_module_t(node->name, mb);
        });
    }
}
