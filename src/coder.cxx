
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
    }

    return true;
}

bool coder_t::encode_type_int(DataStream& stream, struct ast_type_int_t* node)
{
    if(node == nullptr)
        return false;

    stream.write(String("int32_t"));
    
    return true;
}

bool coder_t::encode_expr(DataStream& stream, struct ast_expr_t* node)
{
    if (node == nullptr)
        return false;

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
    stream.write(String("using ") + name + " = ");
    if (!this->encode_type(stream, node->value))
    {
        return false;
    }

    return true;
}

bool coder_t::encode_stmt_symboldef(DataStream& stream, struct ast_stmt_symboldef_t* node)
{
    if (node == nullptr)
        return false;

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
