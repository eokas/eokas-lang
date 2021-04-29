#ifndef _EOKAS_CODER_H_
#define _EOKAS_CODER_H_

#include  "header.h"

_BeginNamespace(eokas)

class coder_t 
{
public:
    coder_t();
    virtual ~coder_t();

public:
    bool encode_module(DataStream& stream, struct ast_module_t* node);

    bool encode_type(DataStream& stream, struct ast_type_t* node);
    bool encode_type_int(DataStream& stream, struct ast_type_int_t* node);
    bool encode_type_float(DataStream& stream, struct ast_type_float_t* node);
    bool encode_type_bool(DataStream& stream, struct ast_type_bool_t* node);
    bool encode_type_string(DataStream& stream, struct ast_type_string_t* node);

    bool encode_expr(DataStream& stream, struct ast_expr_t* node);
    bool encode_expr_trinary(DataStream& stream, struct ast_expr_trinary_t* node);
    bool encode_expr_binary(DataStream& stream, struct ast_expr_binary_t* node);
    bool encode_expr_unary(DataStream& stream, struct ast_expr_unary_t* node);
    bool encode_expr_int(DataStream& stream, struct ast_expr_int_t* node);
    bool encode_expr_float(DataStream& stream, struct ast_expr_float_t* node);
    bool encode_expr_bool(DataStream& stream, struct ast_expr_bool_t* node);

    bool encode_stmt(DataStream& stream, struct ast_stmt_t* node);
    bool encode_stmt_echo(DataStream& stream, struct ast_stmt_echo_t* node);
    bool encode_stmt_typedef(DataStream& stream, struct ast_stmt_typedef_t* node);
    bool encode_stmt_symboldef(DataStream& stream, struct ast_stmt_symboldef_t* node);
    bool encode_stmt_break(DataStream& stream, struct ast_stmt_break_t* node);
    bool encode_stmt_continue(DataStream& stream, struct ast_stmt_continue_t* node);
    bool encode_stmt_return(DataStream& stream, struct ast_stmt_return_t* node);
    bool encode_stmt_if(DataStream& stream, struct ast_stmt_if_t* node);
    bool encode_stmt_while(DataStream& stream, struct ast_stmt_while_t* node);
    bool encode_stmt_for(DataStream& stream, struct ast_stmt_for_t* node);
    bool encode_stmt_block(DataStream& stream, struct ast_stmt_block_t* node);
    bool encode_stmt_assign(DataStream& stream, struct ast_stmt_assign_t* node);
    bool encode_stmt_call(DataStream& stream, struct ast_stmt_call_t* node);

private:
    std::map<String, bool> headers;
};

_EndNamespace(eokas)

#endif//_EOKAS_CODER_H_
