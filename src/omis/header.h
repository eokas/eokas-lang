
#ifndef _EOKAS_OMIS_HEADER_H_
#define _EOKAS_OMIS_HEADER_H_

#include "../ast/ast.h"

namespace eokas {
    template<typename T>
    using predicate_t = std::function<bool(const T&)>;

    using omis_handle_t = void*;

    struct omis_bridge_t;

    class omis_scope_t;
    struct omis_type_symbol_t;
    struct omis_value_symbol_t;

    class omis_module_t;

    class omis_type_t;
    class omis_struct_t;

    class omis_value_t;
    class omis_func_t;

    using omis_loading_t = std::function<omis_module_t*()>;
}

#endif //_EOKAS_OMIS_HEADER_H_