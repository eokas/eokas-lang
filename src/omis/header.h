
#ifndef _EOKAS_OMIS_HEADER_H_
#define _EOKAS_OMIS_HEADER_H_

#include "../ast/ast.h"

namespace eokas {
    template<typename T>
    using predicate_t = std::function<bool(const T&)>;
    using omis_handle_t = void*;

    struct omis_bridge_t;

    class omis_type_t;
    class omis_func_type_t;
    class omis_struct_type_t;

    class omis_value_t;
    class omis_func_t;

    class omis_scope_t;
    class omis_module_t;
}

#endif //_EOKAS_OMIS_HEADER_H_
