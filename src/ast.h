#ifndef _EOKAS_AST_H_
#define _EOKAS_AST_H_

#include "header.h"
#include "scanner.h"

_BeginNamespace(eokas)

struct ast_pos_t
{
    int row;
    int col;

    ast_pos_t() :row(0), col(0) {}
    void set(int r, int c) { this->row = r; this->col = c; }
};

enum class ast_node_category_t
{
    none,

    module,

    type_int,
    type_float,
    type_bool,
    type_string,
    type_ref,

    expr_trinary,
    expr_binary,
    expr_unary,
    expr_int,
    expr_float,
    expr_bool,
    expr_str,
    expr_symbol_ref,

    expr_func_def,
    expr_func_ref,

    expr_array_def,

    expr_obj_def,
    expr_obj_ref,

    expr_index_ref,
    expr_module_ref,

    stmt_echo,
    stmt_break,
    stmt_continue,
    stmt_return,
    stmt_if,
    stmt_while,
    stmt_for,
    stmt_block,
    stmt_assign,
    stmt_call,
    stmt_symbol_def,
    stmt_type_def,
};

enum class ast_unary_oper_t
{
    Pos, Neg, Not, Flip,
    TypeOf, SizeOf,
    Count, Unknown
};

enum class ast_binary_oper_t
{
    Or = 100,
    And = 200,
    Equal = 300, NEqual, LEqual, GEqual, Less, Greater,
    Add = 400, Sub,
    Mul = 500, Div, Mod,
    BitAnd = 600, BitOr, BitXor, ShiftL, ShiftR,
    Is = 700, As,
    MaxPriority,
    Unknown = 0x7FFFFFFF
};


struct ast_node_t
{
    ast_node_category_t category;
    ast_node_t* parent;

    ast_node_t(ast_node_category_t category, ast_node_t* parent)
        : category(category)
        , parent(parent)
    {}

    virtual ~ast_node_t()
    {
        this->category = ast_node_category_t::none;
        this->parent = nullptr;
    }
};

struct ast_type_t :public ast_node_t {
    ast_type_t(ast_node_category_t category, ast_node_t* parent)
        : ast_node_t(category, parent)
    {}
};

struct ast_expr_t :public ast_node_t
{
    ast_expr_t(ast_node_category_t category, ast_node_t* parent)
        : ast_node_t(category, parent)
    {}
};

struct ast_stmt_t :public ast_node_t
{
    ast_stmt_t(ast_node_category_t category, ast_node_t* parent)
        : ast_node_t(category, parent)
    {}
};

struct ast_module_t :public ast_node_t
{
    std::vector<ast_stmt_t*> stmts;

    ast_module_t()
        : ast_node_t(ast_node_category_t::module, nullptr)
    {}

    virtual ~ast_module_t()
    {
        _DeleteList(stmts);
    }
};


struct ast_type_int_t :public ast_type_t {
    ast_type_int_t(ast_node_t* parent)
        : ast_type_t(ast_node_category_t::type_int, parent)
    {}
};

struct ast_type_float_t :public ast_type_t {
    ast_type_float_t(ast_node_t* parent)
        : ast_type_t(ast_node_category_t::type_float, parent)
    {}
};

struct ast_type_bool_t :public ast_type_t {
    ast_type_bool_t(ast_node_t* parent)
        : ast_type_t(ast_node_category_t::type_bool, parent)
    {}
};

struct ast_type_string_t :public ast_type_t {
    ast_type_string_t(ast_node_t* parent)
        : ast_type_t(ast_node_category_t::type_string, parent)
    {}
};

struct ast_type_ref_t :public ast_type_t {
    String name;

    ast_type_ref_t(ast_node_t* parent)
        : ast_type_t(ast_node_category_t::type_ref, parent)
        , name("")
    {}
};


struct ast_expr_trinary_t :public ast_expr_t
{
    ast_expr_t* cond;
    ast_expr_t* branch_true;
    ast_expr_t* branch_false;

    ast_expr_trinary_t(ast_node_t* parent)
        : ast_expr_t(ast_node_category_t::expr_trinary, parent)
        , cond(nullptr)
        , branch_true(nullptr)
        , branch_false(nullptr)
    {}

    virtual ~ast_expr_trinary_t()
    {
        _DeletePointer(cond);
        _DeletePointer(branch_true);
        _DeletePointer(branch_false);
    }
};

struct ast_expr_binary_t :public ast_expr_t
{
    ast_binary_oper_t op;
    ast_expr_t* left;
    ast_expr_t* right;

    ast_expr_binary_t(ast_node_t* parent)
        : ast_expr_t(ast_node_category_t::expr_binary, parent)
        , op(ast_binary_oper_t::Unknown)
        , left(nullptr)
        , right(nullptr)
    {}

    virtual ~ast_expr_binary_t()
    {
        _DeletePointer(left);
        _DeletePointer(right);
    }
};

struct ast_expr_unary_t :public ast_expr_t
{
    ast_unary_oper_t op;
    ast_expr_t* right;

    ast_expr_unary_t(ast_node_t* parent)
        : ast_expr_t(ast_node_category_t::expr_unary, parent)
        , op(ast_unary_oper_t::Unknown)
        , right(nullptr)
    {}

    virtual ~ast_expr_unary_t()
    {
        _DeletePointer(right);
    }
};

struct ast_expr_int_t :public ast_expr_t
{
    i64_t value;

    ast_expr_int_t(ast_node_t* parent)
        : ast_expr_t(ast_node_category_t::expr_int, parent)
        , value(0)
    {}
};

struct ast_expr_float_t :public ast_expr_t
{
    f64_t value;

    ast_expr_float_t(ast_node_t* parent)
        : ast_expr_t(ast_node_category_t::expr_float, parent)
        , value(0)
    {}
};

struct ast_expr_bool_t :public ast_expr_t
{
    bool value;

    ast_expr_bool_t(ast_node_t* parent)
        : ast_expr_t(ast_node_category_t::expr_bool, parent)
        , value(false)
    {}
};

struct ast_expr_str_t :public ast_expr_t
{
    String value;

    ast_expr_str_t(ast_node_t* parent)
        : ast_expr_t(ast_node_category_t::expr_str, parent)
        , value("")
    {}
};

struct ast_expr_symbol_ref_t :public ast_expr_t
{
    String name;

    ast_expr_symbol_ref_t(ast_node_t* parent)
        : ast_expr_t(ast_node_category_t::expr_symbol_ref, parent)
        , name("")
    {}
};

struct ast_expr_func_def_t :public ast_expr_t
{
    ast_type_t* type;
    std::map<String, ast_type_t*> args;
    std::vector<ast_stmt_t*> body;

    ast_expr_func_def_t(ast_node_t* parent)
        : ast_expr_t(ast_node_category_t::expr_func_def, parent)
        , type(nullptr)
        , args()
        , body()
    {}

    virtual ~ast_expr_func_def_t()
    {
        _DeleteMap(args);
        _DeleteList(body);
    }
};

struct ast_expr_func_ref_t :public ast_expr_t
{
    ast_expr_t* func;
    std::vector<ast_expr_t*> args;

    ast_expr_func_ref_t(ast_node_t* parent)
        : ast_expr_t(ast_node_category_t::expr_func_ref, parent)
        , func(nullptr)
        , args()
    {}

    virtual ~ast_expr_func_ref_t()
    {
        _DeletePointer(func);
        _DeleteList(args);
    }
};

struct ast_expr_array_def_t :public ast_expr_t
{
    std::vector<ast_expr_t*> values;

    ast_expr_array_def_t(ast_node_t* parent)
        : ast_expr_t(ast_node_category_t::expr_array_def, parent)
        , values()
    {}

    virtual ~ast_expr_array_def_t()
    {
        _DeleteList(values);
    }
};

struct ast_expr_obj_def_t :public ast_expr_t
{
    struct field_t
    {
        ast_expr_t* value;
        bool alloc;

        field_t()
            :value(nullptr), alloc(true)
        {}

        ~field_t()
        {
            _DeletePointer(this->value);
        }

        void set(ast_expr_t* v, bool a)
        {
            this->value = v;
            this->alloc = a;
        }
    };

    ast_expr_t* base;
    std::map<String, field_t> fields;

    ast_expr_obj_def_t(ast_node_t* parent)
        : ast_expr_t(ast_node_category_t::expr_obj_def, parent)
        , base(nullptr)
        , fields()
    {}

    virtual ~ast_expr_obj_def_t()
    {
        _DeletePointer(base);
    }
};

struct ast_expr_obj_ref_t :public ast_expr_t
{
    ast_expr_t* obj;
    String key;

    ast_expr_obj_ref_t(ast_node_t* parent)
        : ast_expr_t(ast_node_category_t::expr_obj_ref, parent)
        , obj(nullptr)
        , key("")
    {}

    virtual ~ast_expr_obj_ref_t()
    {
        _DeletePointer(obj);
    }
};

struct ast_expr_index_ref_t :public ast_expr_t
{
    ast_expr_t* obj;
    ast_expr_t* key;

    ast_expr_index_ref_t(ast_node_t* parent)
        : ast_expr_t(ast_node_category_t::expr_index_ref, parent)
        , obj(nullptr)
        , key(nullptr)
    {}

    virtual ~ast_expr_index_ref_t()
    {
        _DeletePointer(obj);
        _DeletePointer(key);
    }
};

struct ast_expr_module_ref_t :public ast_expr_t
{
    ast_expr_t* name;

    ast_expr_module_ref_t(ast_node_t* parent)
        : ast_expr_t(ast_node_category_t::expr_module_ref, parent)
        , name(nullptr)
    {}

    virtual ~ast_expr_module_ref_t()
    {
        _DeletePointer(name);
    }
};


struct ast_stmt_echo_t :public ast_stmt_t
{
    std::vector<ast_expr_t*> values;

    ast_stmt_echo_t(ast_node_t* parent)
        : ast_stmt_t(ast_node_category_t::stmt_echo, parent)
        , values()
    {}

    virtual ~ast_stmt_echo_t()
    {
        _DeleteList(values);
    }
};

struct ast_stmt_break_t :public ast_stmt_t
{
    ast_stmt_break_t(ast_node_t* parent)
        : ast_stmt_t(ast_node_category_t::stmt_break, parent)
    {}
};

struct ast_stmt_continue_t :public ast_stmt_t
{
    ast_stmt_continue_t(ast_node_t* parent)
        : ast_stmt_t(ast_node_category_t::stmt_continue, parent)
    {}
};

struct ast_stmt_return_t :public ast_stmt_t
{
    ast_expr_t* value;

    ast_stmt_return_t(ast_node_t* parent)
        : ast_stmt_t(ast_node_category_t::stmt_return, parent)
        , value(nullptr)
    {}

    virtual ~ast_stmt_return_t()
    {
        _DeletePointer(value);
    }
};

struct ast_stmt_if_t :public ast_stmt_t
{
    ast_expr_t* cond;
    ast_stmt_t* branch_true;
    ast_stmt_t* branch_false;

    ast_stmt_if_t(ast_node_t* parent)
        : ast_stmt_t(ast_node_category_t::stmt_if, parent)
        , cond(nullptr)
        , branch_true(nullptr)
        , branch_false(nullptr)
    {}

    virtual ~ast_stmt_if_t()
    {
        _DeletePointer(cond);
        _DeletePointer(branch_true);
        _DeletePointer(branch_false);
    }
};

struct ast_stmt_while_t :public ast_stmt_t
{
    ast_expr_t* cond;
    ast_stmt_t* body;

    ast_stmt_while_t(ast_node_t* parent)
        : ast_stmt_t(ast_node_category_t::stmt_while, parent)
        , cond(nullptr)
        , body(nullptr)
    {}

    virtual ~ast_stmt_while_t()
    {
        _DeletePointer(cond);
        _DeletePointer(body);
    }
};

struct ast_stmt_for_t :public ast_stmt_t
{
    ast_stmt_t* init;
    ast_expr_t* cond;
    ast_stmt_t* step;
    ast_stmt_t* body;

    ast_stmt_for_t(ast_node_t* parent)
        : ast_stmt_t(ast_node_category_t::stmt_for, parent)
        , init(nullptr)
        , cond(nullptr)
        , step(nullptr)
        , body(nullptr)
    {}

    virtual ~ast_stmt_for_t()
    {
        _DeletePointer(init);
        _DeletePointer(cond);
        _DeletePointer(step);
        _DeletePointer(body);
    }
};

struct ast_stmt_block_t :public ast_stmt_t
{
    std::vector<ast_stmt_t*> stmts;

    ast_stmt_block_t(ast_node_t* parent)
        : ast_stmt_t(ast_node_category_t::stmt_block, parent)
        , stmts()
    {}

    virtual ~ast_stmt_block_t()
    {
        _DeleteList(stmts);
    }
};

struct ast_stmt_call_t :public ast_stmt_t
{
    ast_expr_func_ref_t* expr;

    ast_stmt_call_t(ast_node_t* parent)
        : ast_stmt_t(ast_node_category_t::stmt_call, parent)
        , expr(nullptr)
    {}

    virtual ~ast_stmt_call_t()
    {
        _DeletePointer(expr);
    }
};

struct ast_stmt_assign_t :public ast_stmt_t
{
    ast_expr_t* left;
    ast_expr_t* right;

    ast_stmt_assign_t(ast_node_t* parent)
        : ast_stmt_t(ast_node_category_t::stmt_assign, parent)
        , left(nullptr)
        , right(nullptr)
    {}

    virtual ~ast_stmt_assign_t()
    {
        _DeletePointer(left);
        _DeletePointer(right);
    }
};

struct ast_stmt_symbol_def_t :public ast_stmt_t
{
    String name;
    ast_type_t* type;
    ast_expr_t* value;
    bool variable;

    ast_stmt_symbol_def_t(ast_node_t* parent)
        : ast_stmt_t(ast_node_category_t::stmt_symbol_def, parent)
        , name("")
        , type(nullptr)
        , value(nullptr)
        , variable(false)
    {}

    virtual ~ast_stmt_symbol_def_t()
    {
        _DeletePointer(type);
        _DeletePointer(value);
    }
};

struct ast_stmt_type_def_t :public ast_stmt_t
{
    String name;
    ast_type_t* value;

    ast_stmt_type_def_t(ast_node_t* parent)
        : ast_stmt_t(ast_node_category_t::stmt_type_def, parent)
        , name("")
        , value(nullptr)
    {}

    virtual ~ast_stmt_type_def_t()
    {
        _DeletePointer(value);
    }
};

_EndNamespace(eokas)

#endif//_EOKAS_AST_H_
