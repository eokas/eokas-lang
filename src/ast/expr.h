
#ifndef _EOKAS_AST_EXPR_H_
#define _EOKAS_AST_EXPR_H_

#include "header.h"

namespace eokas
{
	struct ast_expr_t : ast_node_t
	{
	    ast_type_t* type;

	    ast_expr_t(ast_node_category_t category, ast_node_t* parent);
	    ~ast_expr_t() override;
	};

	struct ast_expr_trinary_t : public ast_expr_t
	        {
	    ast_expr_t* cond;
	    ast_expr_t* branch_true;
	    ast_expr_t* branch_false;

	    explicit ast_expr_trinary_t(ast_node_t* parent)
	    : ast_expr_t(ast_node_category_t::expr_trinary, parent)
	    , cond(nullptr)
	    , branch_true(nullptr)
	    , branch_false(nullptr)
	    { }

	    ~ast_expr_trinary_t()
	    {
	        _DeletePointer(cond);
	        _DeletePointer(branch_true);
	        _DeletePointer(branch_false);
	    }
	        };

	struct ast_expr_binary_t : public ast_expr_t
	        {
	    ast_binary_oper_t op;

	    ast_expr_binary_t(ast_node_category_t category, ast_node_t* parent)
	    : ast_expr_t(category, parent)
	    , op(ast_binary_oper_t::Unknown)
	    { }
	        };

	struct ast_expr_binary_value_t : public ast_expr_binary_t
	        {
	    ast_expr_t* left;
	    ast_expr_t* right;

	    explicit ast_expr_binary_value_t(ast_node_t* parent)
	    : ast_expr_binary_t(ast_node_category_t::expr_binary_value, parent)
	    , left(nullptr)
	    , right(nullptr)
	    { }

	    ~ast_expr_binary_value_t()
	    {
	        _DeletePointer(left);
	        _DeletePointer(right);
	    }
	        };

	struct ast_expr_binary_type_t : public ast_expr_binary_t
	        {
	    ast_expr_t* left;
	    ast_type_t* right;

	    explicit ast_expr_binary_type_t(ast_node_t* parent)
	    : ast_expr_binary_t(ast_node_category_t::expr_binary_type, parent)
	    , left(nullptr)
	    , right(nullptr)
	    { }

	    ~ast_expr_binary_type_t()
	    {
	        _DeletePointer(left);
	        _DeletePointer(right);
	    }
	        };

	struct ast_expr_unary_t : public ast_expr_t
	        {
	    ast_unary_oper_t op;
	    ast_expr_t* right;

	    explicit ast_expr_unary_t(ast_node_t* parent)
	    : ast_expr_t(ast_node_category_t::expr_unary, parent), op(ast_unary_oper_t::Unknown), right(nullptr)
	    { }

	    ~ast_expr_unary_t()
	    {
	        _DeletePointer(right);
	    }
	        };

	struct ast_expr_int_t : public ast_expr_t
	        {
	    i64_t value;

	    explicit ast_expr_int_t(ast_node_t* parent)
	    : ast_expr_t(ast_node_category_t::expr_int, parent), value(0)
	    { }
	        };

	struct ast_expr_float_t : public ast_expr_t
	        {
	    f64_t value;

	    explicit ast_expr_float_t(ast_node_t* parent)
	    : ast_expr_t(ast_node_category_t::expr_float, parent), value(0)
	    { }
	        };

	struct ast_expr_bool_t : public ast_expr_t
	        {
	    bool value;

	    ast_expr_bool_t(ast_node_t* parent)
	    : ast_expr_t(ast_node_category_t::expr_bool, parent), value(false)
	    { }
	        };

	struct ast_expr_string_t : public ast_expr_t
	        {
	    String value;

	    ast_expr_string_t(ast_node_t* parent)
	    : ast_expr_t(ast_node_category_t::expr_string, parent), value("")
	    { }
	        };

	struct ast_expr_symbol_ref_t : public ast_expr_t
	        {
	    String name;

	    ast_expr_symbol_ref_t(ast_node_t* parent)
	    : ast_expr_t(ast_node_category_t::expr_symbol_ref, parent), name("")
	    { }
	        };

	struct ast_expr_func_def_t : public ast_expr_t
	        {
	    struct arg_t
	            {
	        String name;
	        ast_type_t* type;

	        ~arg_t()
	        {
	            _DeletePointer(type);
	        }
	            };

	    ast_type_t* type;
	    std::vector<arg_t*> args;
	    std::vector<ast_stmt_t*> body;

	    explicit ast_expr_func_def_t(ast_node_t* parent)
	    : ast_expr_t(ast_node_category_t::expr_func_def, parent), type(nullptr), args(), body()
	    { }

	    virtual ~ast_expr_func_def_t()
	    {
	        _DeletePointer(type);
	        _DeleteList(args);
	        _DeleteList(body);
	    }

	    const arg_t* getArg(const String& name) const
	    {
	        for (const auto& arg: args)
	        {
	            if(arg->name == name)
	                return arg;
	        }
	        return nullptr;
	    }

	    void addArg(const String& name, ast_type_t* type)
	    {
	        arg_t* arg = new arg_t();
	        arg->name = name;
	        arg->type = type;
	        this->args.push_back(arg);
	    }
	        };

	struct ast_expr_func_ref_t : public ast_expr_t
	        {
	    ast_expr_t* func;
	    std::vector<ast_expr_t*> args;

	    ast_expr_func_ref_t(ast_node_t* parent)
	    : ast_expr_t(ast_node_category_t::expr_func_ref, parent), func(nullptr), args()
	    { }

	    virtual ~ast_expr_func_ref_t()
	    {
	        _DeletePointer(func);
	        _DeleteList(args);
	    }
	        };

	struct ast_expr_array_def_t : public ast_expr_t
	        {
	    std::vector<ast_expr_t*> items;

	    ast_expr_array_def_t(ast_node_t* parent)
	    : ast_expr_t(ast_node_category_t::expr_array_def, parent), items()
	    { }

	    virtual ~ast_expr_array_def_t()
	    {
	        _DeleteList(items);
	    }
	        };

	struct ast_expr_index_ref_t : public ast_expr_t
	        {
	    ast_expr_t* obj;
	    ast_expr_t* key;

	    ast_expr_index_ref_t(ast_node_t* parent)
	    : ast_expr_t(ast_node_category_t::expr_index_ref, parent), obj(nullptr), key(nullptr)
	    { }

	    virtual ~ast_expr_index_ref_t()
	    {
	        _DeletePointer(obj);
	        _DeletePointer(key);
	    }
	        };

	struct ast_expr_object_def_t : public ast_expr_t
	        {
	    ast_type_t* type;
	    std::map<String, ast_expr_t*> members;

	    ast_expr_object_def_t(ast_node_t* parent)
	    : ast_expr_t(ast_node_category_t::expr_object_def, parent), type(nullptr), members()
	    { }

	    virtual ~ast_expr_object_def_t()
	    {
	        _DeletePointer(type);
	        _DeleteMap(members);
	    }
	        };

	struct ast_expr_object_ref_t : public ast_expr_t
	        {
	    ast_expr_t* obj;
	    String key;

	    ast_expr_object_ref_t(ast_node_t* parent)
	    : ast_expr_t(ast_node_category_t::expr_object_ref, parent), obj(nullptr), key("")
	    { }

	    virtual ~ast_expr_object_ref_t()
	    {
	        _DeletePointer(obj);
	    }
	        };

	struct ast_expr_module_ref_t : public ast_expr_t
	        {
	    ast_expr_t* name;

	    ast_expr_module_ref_t(ast_node_t* parent)
	    : ast_expr_t(ast_node_category_t::expr_module_ref, parent), name(nullptr)
	    { }

	    virtual ~ast_expr_module_ref_t()
	    {
	        _DeletePointer(name);
	    }
	        };

}

#endif //_EOKAS_AST_EXPR_H_
