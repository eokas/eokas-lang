#ifndef _EOKAS_CODER_H_
#define _EOKAS_CODER_H_

#include  "header.h"

_BeginNamespace(eokas)

class coder_t 
{
public:
    coder_t(Stream& stream);
    virtual ~coder_t();

public:
    bool encode_module(struct ast_module_t* node);

    bool encode_type(struct ast_type_t* node);
    bool encode_type_int(struct ast_type_int_t* node);
    bool encode_type_float(struct ast_type_float_t* node);
    bool encode_type_bool(struct ast_type_bool_t* node);
    bool encode_type_string(struct ast_type_string_t* node);
    bool encode_type_ref(struct ast_type_ref_t* node);

    bool encode_expr(struct ast_expr_t* node);
    bool encode_expr_trinary(struct ast_expr_trinary_t* node);
    bool encode_expr_binary(struct ast_expr_binary_t* node);
    bool encode_expr_unary(struct ast_expr_unary_t* node);
    bool encode_expr_int(struct ast_expr_int_t* node);
    bool encode_expr_float(struct ast_expr_float_t* node);
    bool encode_expr_bool(struct ast_expr_bool_t* node);
    bool encode_expr_symbol_ref(struct ast_expr_symbol_ref_t* node);

    bool encode_stmt(struct ast_stmt_t* node);
    bool encode_stmt_echo(struct ast_stmt_echo_t* node);
    bool encode_stmt_typedef(struct ast_stmt_typedef_t* node);
    bool encode_stmt_symboldef(struct ast_stmt_symboldef_t* node);
    bool encode_stmt_break(struct ast_stmt_break_t* node);
    bool encode_stmt_continue(struct ast_stmt_continue_t* node);
    bool encode_stmt_return(struct ast_stmt_return_t* node);
    bool encode_stmt_if(struct ast_stmt_if_t* node);
    bool encode_stmt_while(struct ast_stmt_while_t* node);
    bool encode_stmt_for(struct ast_stmt_for_t* node);
    bool encode_stmt_block(struct ast_stmt_block_t* node);
    bool encode_stmt_assign(struct ast_stmt_assign_t* node);
    bool encode_stmt_call(struct ast_stmt_call_t* node);

private:
    TextStream stream;
    std::map<String, bool> headers;
};

_EndNamespace(eokas)

#endif//_EOKAS_CODER_H_
