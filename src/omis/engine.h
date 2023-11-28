
#ifndef _EOKAS_OMIS_ENGINE_H_
#define _EOKAS_OMIS_ENGINE_H_

#include "./header.h"

namespace eokas {
    class omis_engine_t {
    public:
        omis_engine_t();
        virtual ~omis_engine_t();

        void set_bridge(omis_bridge_t* bridge);
        omis_bridge_t* get_bridge();

        omis_module_t* load_ast(ast_node_module_t* node);

        void jit(omis_module_t* mod);
        void aot(omis_module_t* mod);

    private:
        omis_bridge_t* bridge;
        std::map<String, omis_module_t*> modules;
    };
}

#endif //_EOKAS_OMIS_ENGINE_H_
