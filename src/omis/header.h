
#ifndef _EOKAS_OMIS_HEADER_H_
#define _EOKAS_OMIS_HEADER_H_

#include "../ast/ast.h"

namespace eokas {
    using omis_handle_t = void*;

    struct omis_bridge_t;

    class omis_context_t;
    class omis_module_t;
    class omis_scope_t;
    struct omis_type_symbol_t;
    struct omis_value_symbol_t;

    class omis_type_t;
    class omis_struct_t;

    class omis_value_t;

    template<typename T>
    using omis_lambda_predicate_t = std::function<bool(const T&)>;

    using omis_lambda_expr_t = std::function<omis_value_t*()>;
    using omis_lambda_type_t = std::function<omis_type_t*()>;
    using omis_lambda_stmt_t = std::function<bool()>;

    using omis_lambda_loading_t = std::function<omis_module_t*()>;
}

#endif //_EOKAS_OMIS_HEADER_H_
