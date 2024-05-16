
#ifndef _EOKAS_OMIS_CODER_H_
#define _EOKAS_OMIS_CODER_H_

#include "./model.h"

namespace eokas {
    class omis_module_coder_t :public omis_module_t {
    public:
        omis_module_coder_t(omis_context_t* context, const String& name);

        bool encode_module(ast_node_module_t *node);
        bool encode_import(ast_node_import_t* node);
        bool encode_export(ast_node_export_t* node);
        bool encode_stmt(ast_node_stmt_t* node);
        bool encode_stmt_block(ast_node_block_t* node);
        bool encode_stmt_symbol_def(ast_node_symbol_def_t* node);
        bool encode_stmt_assign(ast_node_assign_t* node);
        bool encode_stmt_return(ast_node_return_t* node);
        bool encode_stmt_if(ast_node_if_t* node);
        bool encode_stmt_loop(ast_node_loop_t* node);
        bool encode_stmt_break(ast_node_break_t* node);
        bool encode_stmt_continue(ast_node_continue_t* node);

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
        omis_value_t* encode_expr_func_def(ast_node_func_def_t* node);
        omis_value_t* encode_expr_func_ref(ast_node_func_ref_t* node);
    };
}

#endif //_EOKAS_OMIS_CODER_H_
