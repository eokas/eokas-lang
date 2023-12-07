
#ifndef _EOKAS_OMIS_ENGINE_H_
#define _EOKAS_OMIS_ENGINE_H_

#include "./header.h"

namespace eokas {
    class omis_engine_t {
    public:
        omis_engine_t();
        virtual ~omis_engine_t();

        omis_bridge_t* get_bridge();

        bool add_module(const String& name, omis_module_t* mod);
        omis_module_t* get_module(const String& name);
        omis_module_t* load_module(const String& name, const omis_lambda_loading_t& loading);

        bool jit(omis_module_t* mod);
        bool aot(omis_module_t* mod);

    private:
        omis_bridge_t* bridge;
        std::map<String, omis_module_t*> modules;
    };
}

#endif //_EOKAS_OMIS_ENGINE_H_
