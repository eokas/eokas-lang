
#ifndef _EOKAS_CODER_H_
#define _EOKAS_CODER_H_

#include "../ast/ast.h"
#include "../omis/context.h"

namespace eokas {
    class coder_t {
    public:
        coder_t();
        ~coder_t();

        omis_module_t* encode(ast_node_module_t* node);
        String dump(omis_module_t* mod);
        void jit(omis_module_t* mod);
        void aot(omis_module_t* mod);

    private:
        omis_context_t* context;
    };
}

#endif //_EOKAS_CODER_H_
