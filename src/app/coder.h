
#ifndef _EOKAS_CODER_H_
#define _EOKAS_CODER_H_

#include "../ast/ast.h"
#include "../omis/engine.h"

namespace eokas {
    class coder_t {
    public:
        coder_t();
        ~coder_t();

        void encode(ast_node_module_t* node);

    private:
        omis_engine_bridge_t* bridge;
        omis_engine_t* engine;
    };
}

#endif //_EOKAS_CODER_H_
