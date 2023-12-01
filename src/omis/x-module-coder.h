
#ifndef _EOKAS_OMIS_CODER_H_
#define _EOKAS_OMIS_CODER_H_

#include "./model.h"

namespace eokas {
    class omis_module_coder_t :public omis_module_t {
    public:
        omis_module_coder_t(omis_bridge_t* bridge, const String& name);

        bool encode_module(ast_node_module_t *node);
        bool encode_stmt(ast_node_stmt_t* node);
        bool encode_stmt_symbol_def(ast_node_symbol_def_t* node);
        bool encode_stmt_return(ast_node_return_t* node);
        bool encode_stmt_if(ast_node_if_t* node);

        omis_type_t* encode_type_ref(ast_node_type_t* node);

        omis_value_t* encode_expr(ast_node_expr_t *node);
        omis_value_t* encode_expr_trinary(struct ast_node_expr_trinary_t *node);
        omis_value_t* encode_expr_binary(ast_node_expr_binary_t *node);
        omis_value_t *encode_expr_unary(ast_node_expr_unary_t *node);
        omis_value_t *encode_expr_int(ast_node_literal_int_t *node);
        omis_value_t *encode_expr_float(ast_node_literal_float_t *node);
        omis_value_t *encode_expr_bool(ast_node_literal_bool_t *node);
        omis_value_t *encode_expr_string(ast_node_literal_string_t *node);
        omis_value_t *encode_expr_symbol_ref(ast_node_symbol_ref_t *node);
    };
}

#endif //_EOKAS_OMIS_CODER_H_
