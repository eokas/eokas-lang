
#include "coder.h"
#include "ast.h"

_BeginNamespace(eokas)

coder_t::coder_t(Stream& stream)
    : stream(TextStream(stream))
    , headers()
{}

coder_t::~coder_t()
{
    this->headers.clear();
}

bool coder_t::encode_module(struct ast_module_t* node)
{
    if (node == nullptr)
        return false;

    for (auto& stmt : node->stmts)
    {
        if (!this->encode_stmt(stmt))
            return false;
    }

    return true;
}

bool coder_t::encode_type(struct ast_type_t* node)
{
    if (node == nullptr)
        return false;

    switch (node->category)
    {
    case ast_node_category_t::type_int:
        return this->encode_type_int(dynamic_cast<ast_type_int_t*>(node));
    case ast_node_category_t::type_float:
        return this->encode_type_float(dynamic_cast<ast_type_float_t*>(node));
    case ast_node_category_t::type_bool:
        return this->encode_type_bool(dynamic_cast<ast_type_bool_t*>(node));
    case ast_node_category_t::type_string:
        return this->encode_type_string(dynamic_cast<ast_type_string_t*>(node));
    case ast_node_category_t::type_ref:
        return this->encode_type_ref(dynamic_cast<ast_type_ref_t*>(node));
    default:
        return false;
    }

    return false;
}

bool coder_t::encode_type_int(struct ast_type_int_t* node)
{
    if (node == nullptr)
        return false;
    stream.write("int");
    return true;
}
bool coder_t::encode_type_float(struct ast_type_float_t* node)
{
    if (node == nullptr)
        return false;
    stream.write("float");
    return true;
}

bool coder_t::encode_type_bool(struct ast_type_bool_t* node)
{
    if (node == nullptr)
        return false;
    stream.write("bool");
    return true;
}

bool coder_t::encode_type_string(struct ast_type_string_t* node)
{
    if (node == nullptr)
        return false;
    stream.write("char*");
    return true;
}

bool coder_t::encode_type_ref(struct ast_type_ref_t* node)
{
    if (node == nullptr)
        return false;
    stream.write(node->name);
    return true;
}

bool coder_t::encode_expr(struct ast_expr_t* node)
{
    if (node == nullptr)
        return false;

    switch (node->category)
    {
    case ast_node_category_t::expr_trinary:
        return this->encode_expr_trinary(dynamic_cast<ast_expr_trinary_t*>(node));
    case ast_node_category_t::expr_binary:
        return this->encode_expr_binary(dynamic_cast<ast_expr_binary_t*>(node));
    case ast_node_category_t::expr_unary:
        return this->encode_expr_unary(dynamic_cast<ast_expr_unary_t*>(node));
    case ast_node_category_t::expr_int:
        return this->encode_expr_int(dynamic_cast<ast_expr_int_t*>(node));
    case ast_node_category_t::expr_float:
        return this->encode_expr_float(dynamic_cast<ast_expr_float_t*>(node));
    case ast_node_category_t::expr_bool:
        return this->encode_expr_bool(dynamic_cast<ast_expr_bool_t*>(node));
    case ast_node_category_t::expr_symbol_ref:
        return this->encode_expr_symbol_ref(dynamic_cast<ast_expr_symbol_ref_t*>(node));
    case ast_node_category_t::expr_func_def:
        return this->encode_expr_func_def(dynamic_cast<ast_expr_func_def_t*>(node));
    case ast_node_category_t::expr_func_ref:
        return this->encode_expr_func_ref(dynamic_cast<ast_expr_func_ref_t*>(node));
    default:
        return false;
    }
    return false;
}

bool coder_t::encode_expr_trinary(struct ast_expr_trinary_t* node)
{
    if (node == nullptr)
        return false;
    stream.write("(");
    if (!this->encode_expr(node->cond))
        return false;
    stream.write(" ? ");
    if (!this->encode_expr(node->branch_true))
        return false;
    stream.write(" : ");
    if (!this->encode_expr(node->branch_false))
        return false;
    stream.write(")");
    return true;
}

bool coder_t::encode_expr_binary(struct ast_expr_binary_t* node)
{
    if (node == nullptr)
        return false;

    stream.write("(");

    if (!this->encode_expr(node->left))
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

    if (!this->encode_expr(node->right))
        return false;

    stream.write(")");

    return true;
}

bool coder_t::encode_expr_unary(struct ast_expr_unary_t* node)
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

    if (!this->encode_expr(node->right))
        return false;

    stream.write(")");

    return true;
}

bool coder_t::encode_expr_int(struct ast_expr_int_t* node)
{
    if (node == nullptr)
        return false;
    stream.write(String::valueToString(node->value));
    return true;
}

bool coder_t::encode_expr_float(struct ast_expr_float_t* node)
{
    if (node == nullptr)
        return false;
    stream.write(String::valueToString(node->value));
    return true;
}

bool coder_t::encode_expr_bool(struct ast_expr_bool_t* node)
{
    if (node == nullptr)
        return false;
    stream.write(String::valueToString(node->value));
    return true;
}

bool coder_t::encode_expr_symbol_ref(struct ast_expr_symbol_ref_t* node)
{
    if (node == nullptr)
        return false;
    stream.write(node->name);
    return true;
}

bool coder_t::encode_expr_func_def(struct ast_expr_func_def_t* node)
{
    if (node == nullptr)
        return false;

    stream.write("[](");
    bool first = true;
    for(auto& pair : node->args)
    {
        if(!first)
            stream.write(", ");
        if(!this->encode_type(pair.second))
            return false;
        const char* name = pair.first.cstr();
        printf("====== %s === \n", name);
        stream.write(String::format(" %s", name));
    }
    stream.write(")");
    if(node->type)
    {
        stream.write("=>");
        if(!this->encode_type(node->type))
            return false;
    }
    stream.write("{\n");
    for(auto& stmt : node->body)
    {
        if(!this->encode_stmt(stmt))
            return false;
    }
    stream.write("}");

    return true;
}

bool coder_t::encode_expr_func_ref(struct ast_expr_func_ref_t* node)
{
    if (node == nullptr)
        return false;
    if (!this->encode_expr(node->func))
        return false;
    stream.write("(");
    bool first = true;
    for (auto& arg : node->args)
    {
        if (!first)
            stream.write(", ");
        first = false;
        if (!this->encode_expr(arg))
            return false;
    }
    stream.write(")");
    return true;
}

bool coder_t::encode_stmt(struct ast_stmt_t* node)
{
    if (node == nullptr)
        return false;

    switch (node->category)
    {
    case ast_node_category_t::stmt_echo:
        return this->encode_stmt_echo(dynamic_cast<ast_stmt_echo_t*>(node));
    case ast_node_category_t::stmt_type_def:
        return this->encode_stmt_type_def(dynamic_cast<ast_stmt_type_def_t*>(node));
    case ast_node_category_t::stmt_symbol_def:
        return this->encode_stmt_symbol_def(dynamic_cast<ast_stmt_symbol_def_t*>(node));
    case ast_node_category_t::stmt_break:
        return this->encode_stmt_break(dynamic_cast<ast_stmt_break_t*>(node));
    case ast_node_category_t::stmt_continue:
        return this->encode_stmt_continue(dynamic_cast<ast_stmt_continue_t*>(node));
    case ast_node_category_t::stmt_return:
        return this->encode_stmt_return(dynamic_cast<ast_stmt_return_t*>(node));
    case ast_node_category_t::stmt_if:
        return this->encode_stmt_if(dynamic_cast<ast_stmt_if_t*>(node));
    case ast_node_category_t::stmt_while:
        return this->encode_stmt_while(dynamic_cast<ast_stmt_while_t*>(node));
    case ast_node_category_t::stmt_for:
        return this->encode_stmt_for(dynamic_cast<ast_stmt_for_t*>(node));
    case ast_node_category_t::stmt_block:
        return this->encode_stmt_block(dynamic_cast<ast_stmt_block_t*>(node));
    case ast_node_category_t::stmt_assign:
        return this->encode_stmt_assign(dynamic_cast<ast_stmt_assign_t*>(node));
    case ast_node_category_t::stmt_call:
        return this->encode_stmt_call(dynamic_cast<ast_stmt_call_t*>(node));
    default:
        return false;
    }

    return false;
}

bool coder_t::encode_stmt_echo(struct ast_stmt_echo_t* node)
{
    if (node == nullptr)
        return false;

    stream.write("std::cout");
    for (auto& v : node->values)
    {
        stream.write("<<(");
        if (!this->encode_expr(v))
        {
            return false;
        }
        stream.write(")");
    }

    this->headers["<iostream>"] = true;

    return true;
}

bool coder_t::encode_stmt_type_def(struct ast_stmt_type_def_t* node)
{
    if (node == nullptr)
        return false;

    const String& name = node->name;
    stream.write(String::format("using %s = ", name.cstr()));
    if (!this->encode_type(node->value))
    {
        return false;
    }
    stream.write(";\n");

    return true;
}

bool coder_t::encode_stmt_symbol_def(struct ast_stmt_symbol_def_t* node)
{
    if (node == nullptr)
        return false;

    if (!node->variable)
    {
        stream.write("const ");
    }

    if (node->type)
    {
        if (!this->encode_type(node->type))
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
        if (!this->encode_expr(node->value))
            return false;
    }

    stream.write(";\n");

    return true;
}

bool coder_t::encode_stmt_break(struct ast_stmt_break_t* node)
{
    if (node == nullptr)
        return false;
    stream.write("break;\n");
    return true;
}

bool coder_t::encode_stmt_continue(struct ast_stmt_continue_t* node)
{
    if (node == nullptr)
        return false;
    stream.write("contine;\n");
    return true;
}

bool coder_t::encode_stmt_return(struct ast_stmt_return_t* node)
{
    if (node == nullptr)
        return false;
    stream.write("return");
    if (node->value)
    {
        stream.write(" ");
        if (!this->encode_expr(node->value))
            return false;
    }
    stream.write(";\n");
    return true;
}

bool coder_t::encode_stmt_if(struct ast_stmt_if_t* node)
{
    if (node == nullptr)
        return false;
    stream.write("if(");
    if (!this->encode_expr(node->cond))
        return false;
    stream.write(")\n");
    if (node->branch_true != nullptr)
    {
        if (!this->encode_stmt(node->branch_true))
            return false;
    }
    if (node->branch_false != nullptr)
    {
        stream.write("\nelse\n");
        if (!this->encode_stmt(node->branch_false))
            return false;
    }
    return true;
}

bool coder_t::encode_stmt_while(struct ast_stmt_while_t* node)
{
    if (node == nullptr)
        return false;

    stream.write("while(");
    if (!this->encode_expr(node->cond))
        return false;
    stream.write(")\n");
    if (!this->encode_stmt(node->body))
        return false;

    return true;
}

bool coder_t::encode_stmt_for(struct ast_stmt_for_t* node)
{
    if (node == nullptr)
        return false;

    stream.write("for(");
    if (!this->encode_stmt(node->init))
        return false;
    if (!this->encode_expr(node->cond))
        return false;
    stream.write("; ");
    if (!this->encode_stmt(node->step))
        return false;
    stream.write(")\n");
    if (!this->encode_stmt(node->body))
        return false;
    return true;
}

bool coder_t::encode_stmt_block(struct ast_stmt_block_t* node)
{
    if (node == nullptr)
        return false;
    stream.write("{\n");
    for (auto& stmt : node->stmts)
    {
        if (!this->encode_stmt(stmt))
            return false;
    }
    stream.write("}\n");
    return true;
}

bool coder_t::encode_stmt_assign(struct ast_stmt_assign_t* node)
{
    if (node == nullptr)
        return false;
    if (!this->encode_expr(node->left))
        return false;
    stream.write(" = ");
    if (!this->encode_expr(node->right))
        return false;
    stream.write(";\n");
    return true;
}

bool coder_t::encode_stmt_call(struct ast_stmt_call_t* node)
{
    if (node == nullptr)
        return false;
    if (!this->encode_expr(node->expr))
        return false;
    stream.write(";\n");
    return true;
}

_EndNamespace(eokas)
