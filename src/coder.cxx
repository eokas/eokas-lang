
#include "coder.h"
#include "ast.h"

_BeginNamespace(eokas)

using A = class {
public:
    void ok() {};
};

A a = A();

coder_t::coder_t()
    : headers()
{}

coder_t::~coder_t()
{
    this->headers.clear();
}

bool coder_t::encode_module(DataStream& stream, struct ast_module_t* node)
{
    if (node == nullptr)
        return false;

    for (auto& stmt : node->stmts)
    {
        if (!this->encode_stmt(stream, stmt))
            return false;
    }

    return true;
}

bool coder_t::encode_type(DataStream& stream, struct ast_type_t* node)
{
    if (node == nullptr)
        return false;

    switch (node->category)
    {
    case ast_node_category_t::type_int:
        return this->encode_type_int(stream, dynamic_cast<ast_type_int_t*>(node));
    case ast_node_category_t::type_float:
        return this->encode_type_float(stream, dynamic_cast<ast_type_float_t*>(node));
    case ast_node_category_t::type_bool:
        return this->encode_type_bool(stream, dynamic_cast<ast_type_bool_t*>(node));
    case ast_node_category_t::type_string:
        return this->encode_type_string(stream, dynamic_cast<ast_type_string_t*>(node));
    default:
        return false;
    }

    return false;
}

bool coder_t::encode_type_int(DataStream& stream, struct ast_type_int_t* node)
{
    if (node == nullptr)
        return false;
    stream.write("int32_t");
    return true;
}
bool coder_t::encode_type_float(DataStream& stream, struct ast_type_float_t* node)
{
    if (node == nullptr)
        return false;
    stream.write("float");
    return true;
}

bool coder_t::encode_type_bool(DataStream& stream, struct ast_type_bool_t* node)
{
    if (node == nullptr)
        return false;
    stream.write("bool");
    return true;
}

bool coder_t::encode_type_string(DataStream& stream, struct ast_type_string_t* node)
{
    if (node == nullptr)
        return false;
    stream.write("char*");
    return true;
}

bool coder_t::encode_expr(DataStream& stream, struct ast_expr_t* node)
{
    if (node == nullptr)
        return false;

    switch (node->category)
    {
    case ast_node_category_t::expr_trinary:
        return this->encode_expr_trinary(stream, dynamic_cast<ast_expr_trinary_t*>(node));
    case ast_node_category_t::expr_binary:
        return this->encode_expr_binary(stream, dynamic_cast<ast_expr_binary_t*>(node));
    case ast_node_category_t::expr_unary:
        return this->encode_expr_unary(stream, dynamic_cast<ast_expr_unary_t*>(node));
    case ast_node_category_t::expr_int:
        return this->encode_expr_int(stream, dynamic_cast<ast_expr_int_t*>(node));
    case ast_node_category_t::expr_float:
        return this->encode_expr_float(stream, dynamic_cast<ast_expr_float_t*>(node));
    case ast_node_category_t::expr_bool:
        return this->encode_expr_bool(stream, dynamic_cast<ast_expr_bool_t*>(node));
    default:
        return false;
    }
    return false;
}

bool coder_t::encode_expr_trinary(DataStream& stream, struct ast_expr_trinary_t* node)
{
    if (node == nullptr)
        return false;
    stream.write("(");
    if (!this->encode_expr(stream, node->cond))
        return false;
    stream.write(" ? ");
    if (!this->encode_expr(stream, node->branch_true))
        return false;
    stream.write(" : ");
    if (!this->encode_expr(stream, node->branch_false))
        return false;
    stream.write(")");
    return true;
}

bool coder_t::encode_expr_binary(DataStream& stream, struct ast_expr_binary_t* node)
{
    if (node == nullptr)
        return false;

    stream.write("(");

    if (!this->encode_expr(stream, node->left))
        return false;

    switch (node->op)
    {
    case ast_binary_oper_t::Add:stream.write(" + "); break;
    case ast_binary_oper_t::Sub:stream.write(" - "); break;
    case ast_binary_oper_t::Mul:stream.write(" * "); break;
    case ast_binary_oper_t::Div:stream.write(" / "); break;
    case ast_binary_oper_t::Mod: stream.write(" % "); break;
    default:
        return false;
    }

    if (!this->encode_expr(stream, node->right))
        return false;

    stream.write(")");

    return true;
}

bool coder_t::encode_expr_unary(DataStream& stream, struct ast_expr_unary_t* node)
{
    if (node == nullptr)
        return false;

    stream.write("(");

    switch (node->op)
    {
    case ast_unary_oper_t::Pos:stream.write(" + "); break;
    case ast_unary_oper_t::Neg:stream.write(" - "); break;
    default:
        return false;
    }

    if (!this->encode_expr(stream, node->right))
        return false;

    stream.write(")");
}

bool coder_t::encode_expr_int(DataStream& stream, struct ast_expr_int_t* node)
{
    if (node == nullptr)
        return false;
    stream.write(String::valueToString(node->value));
    return true;
}

bool coder_t::encode_expr_float(DataStream& stream, struct ast_expr_float_t* node)
{
    if (node == nullptr)
        return false;
    stream.write(String::valueToString(node->value));
    return true;
}

bool coder_t::encode_expr_bool(DataStream& stream, struct ast_expr_bool_t* node)
{
    if (node == nullptr)
        return false;
    stream.write(String::valueToString(node->value));
    return true;
}

bool coder_t::encode_stmt(DataStream& stream, struct ast_stmt_t* node)
{
    if (node == nullptr)
        return false;

    switch (node->category)
    {
    case ast_node_category_t::stmt_echo:
        return this->encode_stmt_echo(stream, dynamic_cast<ast_stmt_echo_t*>(node));
    case ast_node_category_t::stmt_typedef:
        return this->encode_stmt_typedef(stream, dynamic_cast<ast_stmt_typedef_t*>(node));
    case ast_node_category_t::stmt_symboldef:
        return this->encode_stmt_symboldef(stream, dynamic_cast<ast_stmt_symboldef_t*>(node));
    case ast_node_category_t::stmt_break:
        return this->encode_stmt_break(stream, dynamic_cast<ast_stmt_break_t*>(node));
    case ast_node_category_t::stmt_continue:
        return this->encode_stmt_continue(stream, dynamic_cast<ast_stmt_continue_t*>(node));
    case ast_node_category_t::stmt_return:
        return this->encode_stmt_return(stream, dynamic_cast<ast_stmt_return_t*>(node));
    case ast_node_category_t::stmt_if:
        return this->encode_stmt_if(stream, dynamic_cast<ast_stmt_if_t*>(node));
    case ast_node_category_t::stmt_while:
        return this->encode_stmt_while(stream, dynamic_cast<ast_stmt_while_t*>(node));
    case ast_node_category_t::stmt_for:
        return this->encode_stmt_for(stream, dynamic_cast<ast_stmt_for_t*>(node));
    case ast_node_category_t::stmt_block:
        return this->encode_stmt_block(stream, dynamic_cast<ast_stmt_block_t*>(node));
    case ast_node_category_t::stmt_assign:
        return this->encode_stmt_assign(stream, dynamic_cast<ast_stmt_assign_t*>(node));
    case ast_node_category_t::stmt_call:
        return this->encode_stmt_call(stream, dynamic_cast<ast_stmt_call_t*>(node));
    default:
        return false;
    }

    return false;
}

bool coder_t::encode_stmt_echo(DataStream& stream, struct ast_stmt_echo_t* node)
{
    if (node == nullptr)
        return false;

    stream.write("std::cout");
    for (auto& v : node->values)
    {
        stream.write("<<(");
        if (!this->encode_expr(stream, v))
        {
            return false;
        }
        stream.write(")");
    }

    this->headers["<iostream>"] = true;

    return true;
}

bool coder_t::encode_stmt_typedef(DataStream& stream, struct ast_stmt_typedef_t* node)
{
    if (node == nullptr)
        return false;

    const String& name = node->name;
    stream.write(String::format("using %s = ", name.cstr()));
    if (!this->encode_type(stream, node->value))
    {
        return false;
    }
    stream.write(";");

    return true;
}

bool coder_t::encode_stmt_symboldef(DataStream& stream, struct ast_stmt_symboldef_t* node)
{
    if (node == nullptr)
        return false;

    if(! node->variable)
    {
        stream.write("const ");
    }

    if (node->type)
    {
        if (!this->encode_type(stream, node->type))
            return false;
    }
    else {
        stream.write("auto");
    }

    const String& name = node->name;
    stream.write(String::format(" %s", name.cstr()));

    if (node->value)
    {
        stream.write(" = ");
        if (!this->encode_expr(stream, node->value))
            return false;
    }

    stream.write(";");

    return true;
}

bool coder_t::encode_stmt_break(DataStream& stream, struct ast_stmt_break_t* node)
{
    if (node == nullptr)
        return false;

    return true;
}

bool coder_t::encode_stmt_continue(DataStream& stream, struct ast_stmt_continue_t* node)
{
    if (node == nullptr)
        return false;

    return true;
}

bool coder_t::encode_stmt_return(DataStream& stream, struct ast_stmt_return_t* node)
{
    if (node == nullptr)
        return false;

    return true;
}

bool coder_t::encode_stmt_if(DataStream& stream, struct ast_stmt_if_t* node)
{
    if (node == nullptr)
        return false;

    return true;
}

bool coder_t::encode_stmt_while(DataStream& stream, struct ast_stmt_while_t* node)
{
    if (node == nullptr)
        return false;

    return true;
}

bool coder_t::encode_stmt_for(DataStream& stream, struct ast_stmt_for_t* node)
{
    if (node == nullptr)
        return false;

    return true;
}

bool coder_t::encode_stmt_block(DataStream& stream, struct ast_stmt_block_t* node)
{
    if (node == nullptr)
        return false;

    return true;
}

bool coder_t::encode_stmt_assign(DataStream& stream, struct ast_stmt_assign_t* node)
{
    if (node == nullptr)
        return false;

    return true;
}

bool coder_t::encode_stmt_call(DataStream& stream, struct ast_stmt_call_t* node)
{
    if (node == nullptr)
        return false;

    return true;
}

_EndNamespace(eokas)
