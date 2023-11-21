
#ifndef _EOKAS_OMIS_HEADER_H_
#define _EOKAS_OMIS_HEADER_H_

#include "../ast/ast.h"

namespace eokas {
    template<typename T>
    using predicate_t = std::function<bool(const T&)>;

    struct omis_type_t;
    struct omis_func_type_t;
    struct omis_struct_type_t;

    struct omis_value_t;
    struct omis_func_t;

    struct omis_scope_t;
    struct omis_module_t;
}

#endif //_EOKAS_OMIS_HEADER_H_
