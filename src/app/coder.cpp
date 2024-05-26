
#include "./coder.h"
#include "../omis/model.h"
#include "../omis/x-module-coder.h"

namespace eokas {
    coder_t::coder_t() {
        context = new omis_context_t();
        context->load_default_modules();
    }

    coder_t::~coder_t() {
        _DeletePointer(context);
    }

    omis_module_t* coder_t::encode(ast_node_module_t* node) {
        return context->load_module(node->name, [&]() -> omis_module_t* {
            auto mod = new omis_module_coder_t(context, node->name);
            if(!mod->encode_module(node)) {
                _DeletePointer(mod);
                return nullptr;
            }
            return mod;
        });
    }

    String coder_t::dump(omis_module_t *mod) {
        return mod->dump();
    }

    void coder_t::jit(omis_module_t* mod) {
        context->jit(mod);
    }

    void coder_t::aot(omis_module_t* mod) {
        context->aot(mod);
    }
}
