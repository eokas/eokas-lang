
#include "coder.h"
#include "ast.h"

_BeginNamespace(eokas)

struct coder_cxx_t
{
    Stream& base;

    MemoryStream header;
    MemoryStream global;
    MemoryStream local;
    TextStream stream;

    int counter;

    coder_cxx_t(Stream& stream)
        : base(stream)
        , header()
        , global()
        , local()
        , stream(local)
        , counter(0)
    { }

    bool encode(struct ast_module_t* m)
    {
        header.open();
        global.open();
        local.open();

        stream.bind(local);
        if (!this->encode_module(m))
            return false;

        String h;
        header.seek(0, 0);
        stream.bind(header);
        stream.read(h);
        printf("H: %s\n", h.cstr());

        String g;
        global.seek(0, 0);
        stream.bind(global);
        stream.read(g);
        printf("G: %s\n", g.cstr());

        String l;
        local.seek(0, 0);
        stream.bind(local);
        stream.read(l);
        printf("L: %s\n", l.cstr());

        stream.bind(base);
        stream.write(h);
        stream.write("\n");
        stream.write(g);
        stream.write("\n");
        stream.write(l);
        stream.flush();

        return true;
    }

    bool encode_module(struct ast_module_t* node)
    {
        if (node == nullptr)
            return false;

        stream.write("void module_main()\n{\n");
        for (auto& stmt : node->stmts)
        {
            if (!this->encode_stmt(stmt))
                return false;
        }
        stream.write("}\n");
        return true;
    }

    bool encode_type(struct ast_type_t* node)
    {
        if (node == nullptr)
            return false;

        switch (node->category)
        {
        case ast_node_category_t::type_ref:
            return this->encode_type_ref(dynamic_cast<ast_type_ref_t*>(node));
        default:
            return false;
        }

        return false;
    }

    bool encode_type_ref(struct ast_type_ref_t* node)
    {
        if (node == nullptr)
            return false;

        const String& name = node->name;
        if (name == "int" ||
            name == "float" ||
            name == "bool")
        {
            stream.write(node->name);
            return true;
        }
        else if (name == "string")
        {
            stream.write("const char*");
            return true;
        }

        return false;
    }

    bool encode_expr(struct ast_expr_t* node)
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
        case ast_node_category_t::expr_string:
            return this->encode_expr_string(dynamic_cast<ast_expr_string_t*>(node));
        case ast_node_category_t::expr_symbol_ref:
            return this->encode_expr_symbol_ref(dynamic_cast<ast_expr_symbol_ref_t*>(node));
        case ast_node_category_t::expr_func_def:
            return this->encode_expr_func_def(dynamic_cast<ast_expr_func_def_t*>(node));
        case ast_node_category_t::expr_func_ref:
            return this->encode_expr_func_ref(dynamic_cast<ast_expr_func_ref_t*>(node));
        case ast_node_category_t::expr_array_def:
            return this->encode_expr_array_def(dynamic_cast<ast_expr_array_def_t*>(node));
        case ast_node_category_t::expr_index_ref:
            return this->encode_expr_index_ref(dynamic_cast<ast_expr_index_ref_t*>(node));
        default:
            return false;
        }
        return false;
    }

    bool encode_expr_trinary(struct ast_expr_trinary_t* node)
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

    bool encode_expr_binary(struct ast_expr_binary_t* node)
    {
        if (node == nullptr)
            return false;

        stream.write("(");

        if (!this->encode_expr(node->left))
            return false;

        switch (node->op)
        {
        case ast_binary_oper_t::Or: stream.write(" || "); break;
        case ast_binary_oper_t::And: stream.write(" && "); break;
        case ast_binary_oper_t::Equal: stream.write(" == "); break;
        case ast_binary_oper_t::NEqual: stream.write(" != "); break;
        case ast_binary_oper_t::LEqual: stream.write(" <= "); break;
        case ast_binary_oper_t::GEqual: stream.write(" >= "); break;
        case ast_binary_oper_t::Less: stream.write(" < "); break;
        case ast_binary_oper_t::Greater: stream.write(" > "); break;
        case ast_binary_oper_t::Add:stream.write(" + "); break;
        case ast_binary_oper_t::Sub:stream.write(" - "); break;
        case ast_binary_oper_t::Mul:stream.write(" * "); break;
        case ast_binary_oper_t::Div:stream.write(" / "); break;
        case ast_binary_oper_t::Mod: stream.write(" % "); break;
        case ast_binary_oper_t::BitAnd:stream.write(" & "); break;
        case ast_binary_oper_t::BitOr:stream.write(" | "); break;
        case ast_binary_oper_t::BitXor:stream.write(" ^ "); break;
        case ast_binary_oper_t::ShiftL:stream.write(" << "); break;
        case ast_binary_oper_t::ShiftR:stream.write(" >> "); break;
        default:
            return false;
        }

        if (!this->encode_expr(node->right))
            return false;

        stream.write(")");

        return true;
    }

    bool encode_expr_unary(struct ast_expr_unary_t* node)
    {
        if (node == nullptr)
            return false;

        stream.write("(");

        switch (node->op)
        {
        case ast_unary_oper_t::Pos:stream.write(" + "); break;
        case ast_unary_oper_t::Neg:stream.write(" - "); break;
        case ast_unary_oper_t::Not:stream.write(" ! "); break;
        case ast_unary_oper_t::Flip:stream.write(" ~ "); break;
        default:
            return false;
        }

        if (!this->encode_expr(node->right))
            return false;

        stream.write(")");

        return true;
    }

    bool encode_expr_int(struct ast_expr_int_t* node)
    {
        if (node == nullptr)
            return false;
        stream.write(String::valueToString(node->value));
        return true;
    }

    bool encode_expr_float(struct ast_expr_float_t* node)
    {
        if (node == nullptr)
            return false;
        stream.write(String::valueToString(node->value));
        return true;
    }

    bool encode_expr_bool(struct ast_expr_bool_t* node)
    {
        if (node == nullptr)
            return false;
        stream.write(String::valueToString(node->value));
        return true;
    }

    bool encode_expr_string(struct ast_expr_string_t* node)
    {
        if (node == nullptr)
            return false;
        stream.write(String::format("\"%s\"", node->value.cstr()));
        return true;
    }

    bool encode_expr_symbol_ref(struct ast_expr_symbol_ref_t* node)
    {
        if (node == nullptr)
            return false;
        stream.write(node->name);
        return true;
    }

    bool encode_expr_func_def(struct ast_expr_func_def_t* node)
    {
        if (node == nullptr)
            return false;

        Stream& oldTarget = stream.target();
        stream.bind(global);
        String globalName = String::format("func_%d", counter++);
        stream.write(String::format("struct %s{ \n", globalName.cstr()));
        stream.write("auto operator()(");
        bool first = true;
        for (auto& pair : node->args)
        {
            if (!first)
                stream.write(", ");
            if (!this->encode_type(pair.second))
                return false;
            const char* name = pair.first.cstr();
            stream.write(String::format(" %s", name));
        }
        stream.write(") const");
        if (node->type)
        {
            stream.write("->");
            if (!this->encode_type(node->type))
                return false;
        }
        stream.write("{\n");
        for (auto& stmt : node->body)
        {
            if (!this->encode_stmt(stmt))
                return false;
        }
        stream.write("}\n");
        stream.write("};\n");

        stream.bind(oldTarget);
        stream.write(String::format("%s()", globalName.cstr()));

        return true;
    }

    bool encode_expr_func_ref(struct ast_expr_func_ref_t* node)
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

    bool encode_expr_array_def(struct ast_expr_array_def_t* node)
    {
        if (node == nullptr)
            return false;
        stream.write("{");
        bool first = true;
        for (auto& item : node->items)
        {
            if (!first)
                stream.write(", ");
            first = false;
            if (!this->encode_expr(item))
                return false;
        }
        stream.write("}");
        return true;
    }

    bool encode_expr_index_ref(struct ast_expr_index_ref_t* node)
    {
        if (node == nullptr)
            return false;
        if (!this->encode_expr(node->obj))
            return false;
        stream.write("[");
        if (!this->encode_expr(node->key))
            return false;
        stream.write("]");
        return true;
    }

    bool encode_stmt(struct ast_stmt_t* node)
    {
        if (node == nullptr)
            return false;

        switch (node->category)
        {
        case ast_node_category_t::stmt_echo:
            return this->encode_stmt_echo(dynamic_cast<ast_stmt_echo_t*>(node));
        case ast_node_category_t::stmt_schema_def:
            return this->encode_stmt_schema_def(dynamic_cast<ast_stmt_schema_def_t*>(node));
        case ast_node_category_t::stmt_struct_def:
            return this->encode_stmt_struct_def(dynamic_cast<ast_stmt_struct_def_t*>(node));
        case ast_node_category_t::stmt_proc_def:
            return this->encode_stmt_proc_def(dynamic_cast<ast_stmt_proc_def_t*>(node));
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

    bool encode_stmt_echo(struct ast_stmt_echo_t* node)
    {
        if (node == nullptr)
            return false;

        Stream& oldTarget = stream.target();
        stream.bind(header);
        stream.write("#include <iostream>\n");

        stream.bind(oldTarget);
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
        stream.write(";\n");

        return true;
    }

    bool encode_stmt_schema_def(struct ast_stmt_schema_def_t* node)
    {
        if (node == nullptr)
            return false;

        const String& name = node->name;
        stream.write(String::format("struct %s", name.cstr()));
        if (node->schema != nullptr)
        {
            stream.write(" : ");
            if (!this->encode_type(node->schema))
                return false;
        }
        stream.write("{\n");
        for (auto& m : node->members)
        {
            auto type = m.second->type;
            if (!this->encode_type(type))
                return false;
            stream.write(" ");
            stream.write(m.first);
            stream.write(";\n");
        }
        stream.write("};\n");

        return true;
    }

    bool encode_stmt_struct_def(struct ast_stmt_struct_def_t* node)
    {
        if (node == nullptr)
            return false;

        const String& name = node->name;
        stream.write(String::format("struct %s", name.cstr()));
        if (node->schema != nullptr)
        {
            stream.write(" : ");
            if (!this->encode_type(node->schema))
                return false;
        }
        stream.write("{\n");
        for (auto& m : node->members)
        {
            auto type = m.second->type;
            auto value = m.second->value;

            if (!this->encode_type(type))
                return false;

            stream.write(" ");
            stream.write(m.first);
            if (value != nullptr)
            {
                stream.write(" = ");
                if (!this->encode_expr(value))
                    return false;
            }
            stream.write(";\n");
        }
        stream.write("};\n");

        return true;
    }

    bool encode_stmt_proc_def(struct ast_stmt_proc_def_t* node)
    {
        if (node == nullptr)
            return false;

        Stream& oldTarget = stream.target();
        stream.bind(global);
        String globalName = String::format("proc_%d", counter++);
        stream.write(String::format("struct %s{ \n", globalName.cstr()));
        stream.write("virtual auto operator()(");
        bool first = true;
        for (auto& pair : node->args)
        {
            if (!first)
                stream.write(", ");
            if (!this->encode_type(pair.second))
                return false;
            const char* name = pair.first.cstr();
            stream.write(String::format(" %s", name));
        }
        stream.write(") const");
        if (node->type)
        {
            stream.write("->");
            if (!this->encode_type(node->type))
                return false;
            stream.write("= 0;\n");
        }
        stream.write("};\n");

        stream.bind(oldTarget);
        stream.write(String::format("%s()", globalName.cstr()));

        return true;
    }

    bool encode_stmt_symbol_def(struct ast_stmt_symbol_def_t* node)
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

    bool encode_stmt_break(struct ast_stmt_break_t* node)
    {
        if (node == nullptr)
            return false;
        stream.write("break;\n");
        return true;
    }

    bool encode_stmt_continue(struct ast_stmt_continue_t* node)
    {
        if (node == nullptr)
            return false;
        stream.write("contine;\n");
        return true;
    }

    bool encode_stmt_return(struct ast_stmt_return_t* node)
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

    bool encode_stmt_if(struct ast_stmt_if_t* node)
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

    bool encode_stmt_while(struct ast_stmt_while_t* node)
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

    bool encode_stmt_for(struct ast_stmt_for_t* node)
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

    bool encode_stmt_block(struct ast_stmt_block_t* node)
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

    bool encode_stmt_assign(struct ast_stmt_assign_t* node)
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

    bool encode_stmt_call(struct ast_stmt_call_t* node)
    {
        if (node == nullptr)
            return false;
        if (!this->encode_expr(node->expr))
            return false;
        stream.write(";\n");
        return true;
    }
};


coder_t::coder_t(Stream& stream)
    : impl(new coder_cxx_t(stream))
{}

coder_t::~coder_t()
{
    _DeletePointer(impl);
}

bool coder_t::encode(struct ast_module_t* m)
{
    return impl->encode(m);
}

_EndNamespace(eokas)
