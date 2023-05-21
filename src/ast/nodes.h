
#ifndef _EOKAS_AST_NODES_H_
#define _EOKAS_AST_NODES_H_

#include "header.h"

namespace eokas
{
	struct ast_node_t
	{
		ast_category_t category;
		ast_node_t* parent;

		explicit ast_node_t(ast_category_t category, ast_node_t* parent)
			: category(category), parent(parent)
		{}

		virtual ~ast_node_t()
		{
			this->category = ast_category_t::NONE;
			this->parent = nullptr;
		}
	};
	
	struct ast_node_module_t : public ast_node_t
	{
		String name = "";
		std::map<String, ast_node_import_t*> imports = {};
		std::map<String, ast_node_export_t*> exports = {};
		ast_node_func_def_t* entry = nullptr;

		explicit ast_node_module_t(ast_node_t* parent)
			: ast_node_t(ast_category_t::MODULE, parent)
		{}
	};

	struct ast_node_import_t : public ast_node_t
	{
		String name = "";
		String target = "";

		explicit ast_node_import_t(ast_node_t* parent)
			: ast_node_t(ast_category_t::IMPORT, parent)
		{}
	};

	struct ast_node_export_t : public ast_node_t
	{
		String name = "";
		
		explicit ast_node_export_t(ast_node_t* parent)
			: ast_node_t(ast_category_t::EXPORT, parent)
		{}
	};

	struct ast_node_type_t : public ast_node_t
	{
		String name = "";
		std::vector<ast_node_type_t*> args = {};
		
		explicit ast_node_type_t(ast_node_t* parent)
			: ast_node_t(ast_category_t::TYPE, parent)
		{}
	};

	struct ast_node_expr_t : public ast_node_t
	{
		explicit ast_node_expr_t(ast_category_t category, ast_node_t* parent)
			: ast_node_t(category, parent)
		{}
	};

	struct ast_node_stmt_t : public ast_node_t
	{
		explicit ast_node_stmt_t(ast_category_t category, ast_node_t* parent)
			: ast_node_t(category, parent)
		{}
	};

	struct ast_node_func_def_t : public ast_node_expr_t
	{
		struct arg_t
		{
			String name = "";
			ast_node_type_t* type = nullptr;
		};

		ast_node_type_t* rtype = nullptr;
		std::vector<arg_t> args = {};
		std::vector<ast_node_stmt_t*> body = {};

		explicit ast_node_func_def_t(ast_node_t* parent)
			: ast_node_expr_t(ast_category_t::FUNC_DEF, parent)
		{}
		
		arg_t* addArg(const String& name)
		{
			if(this->getArg(name) != nullptr)
				return nullptr;
			auto& arg = this->args.emplace_back();
			arg.name = name;
			return &arg;
		}
		
		const arg_t* getArg(const String& name) const
		{
			for(auto& arg : args)
			{
				if(arg.name == name)
					return &arg;
			}
			return nullptr;
		}
	};

	struct ast_node_func_ref_t : public ast_node_expr_t
	{
		ast_node_expr_t* func = nullptr;
		std::vector<ast_node_expr_t*> args = {};

		explicit ast_node_func_ref_t(ast_node_t* parent)
			: ast_node_expr_t(ast_category_t::FUNC_REF, parent)
		{}
	};

	struct ast_node_symbol_def_t : public ast_node_stmt_t
	{
		String name = "";
		ast_node_type_t* type = nullptr;
		ast_node_expr_t* value = nullptr;
		bool variable = false;

		explicit ast_node_symbol_def_t(ast_node_t* parent)
			: ast_node_stmt_t(ast_category_t::SYMBOL_DEF, parent)
		{}
	};

	struct ast_node_symbol_ref_t : public ast_node_expr_t
	{
		String name = "";

		explicit ast_node_symbol_ref_t(ast_node_t* parent)
			: ast_node_expr_t(ast_category_t::SYMBOL_REF, parent)
		{}
	};

	struct ast_node_expr_trinary_t : public ast_node_expr_t
	{
		ast_node_expr_t* cond = nullptr;
		ast_node_expr_t* branch_true = nullptr;
		ast_node_expr_t* branch_false = nullptr;

		explicit ast_node_expr_trinary_t(ast_node_t* parent)
			: ast_node_expr_t(ast_category_t::EXPR_TRINARY, parent)
		{}
	};

	struct ast_node_expr_binary_t : public ast_node_expr_t
	{
		ast_binary_oper_t op = ast_binary_oper_t::UNKNOWN;
		ast_node_expr_t* left = nullptr;
		ast_node_expr_t* right = nullptr;

		explicit ast_node_expr_binary_t(ast_node_t* parent)
			: ast_node_expr_t(ast_category_t::EXPR_BINARY, parent)
		{}
	};

	struct ast_node_expr_unary_t : public ast_node_expr_t
	{
		ast_unary_oper_t op = ast_unary_oper_t::UNKNOWN;
		ast_node_expr_t* right = nullptr;

		explicit ast_node_expr_unary_t(ast_node_t* parent)
			: ast_node_expr_t(ast_category_t::EXPR_UNARY, parent)
		{}
	};

	struct ast_node_literal_int_t : public ast_node_expr_t
	{
		i64_t value = 0;

		explicit ast_node_literal_int_t(ast_node_t* parent)
			: ast_node_expr_t(ast_category_t::LITERAL_INT, parent)
		{}
	};

	struct ast_node_literal_float_t : public ast_node_expr_t
	{
		f64_t value = 0;

		explicit ast_node_literal_float_t(ast_node_t* parent)
			: ast_node_expr_t(ast_category_t::LITERAL_FLOAT, parent)
		{}
	};

	struct ast_node_literal_bool_t : public ast_node_expr_t
	{
		bool value = false;

		explicit ast_node_literal_bool_t(ast_node_t* parent)
			: ast_node_expr_t(ast_category_t::LITERAL_BOOL, parent)
		{}
	};

	struct ast_node_literal_string_t : public ast_node_expr_t
	{
		String value = "";

		explicit ast_node_literal_string_t(ast_node_t* parent)
			: ast_node_expr_t(ast_category_t::LITERAL_STRING, parent)
		{}
	};

	struct ast_node_array_def_t : public ast_node_expr_t
	{
		std::vector<ast_node_expr_t*> elements = {};

		explicit ast_node_array_def_t(ast_node_t* parent)
			: ast_node_expr_t(ast_category_t::ARRAY_DEF, parent)
		{}
	};

	struct ast_node_array_ref_t : public ast_node_expr_t
	{
		ast_node_expr_t* obj = nullptr;
		ast_node_expr_t* key = nullptr;

		explicit ast_node_array_ref_t(ast_node_t* parent)
			: ast_node_expr_t(ast_category_t::ARRAY_REF, parent)
		{}
	};

	struct ast_node_object_def_t : public ast_node_expr_t
	{
		ast_node_type_t* type = nullptr;
		std::map<String, ast_node_expr_t*> members = {};

		explicit ast_node_object_def_t(ast_node_t* parent)
			: ast_node_expr_t(ast_category_t::OBJECT_DEF, parent)
		{}
	};

	struct ast_node_object_ref_t : public ast_node_expr_t
	{
		ast_node_expr_t* obj = nullptr;
		String key = "";

		explicit ast_node_object_ref_t(ast_node_t* parent)
			: ast_node_expr_t(ast_category_t::OBJECT_REF, parent)
		{}
	};

	struct ast_node_struct_def_t : public ast_node_stmt_t
	{
		struct member_t
		{
			String name = "";
			ast_node_type_t* type = nullptr;
			ast_node_expr_t* value = nullptr;
			bool isConst = false;
		};

		String name = "";
		std::vector<member_t> members = {};

		explicit ast_node_struct_def_t(ast_node_t* parent)
			: ast_node_stmt_t(ast_category_t::STRUCT_DEF, parent)
		{ }
		
		member_t* addMember(const String& name)
		{
			if(this->getMember(name) != nullptr)
				return nullptr;
			
			auto& m = this->members.emplace_back();
			m.name = name;
			return &m;
		}
		
		const member_t* getMember(const String& name) const
		{
			for(auto& m : members)
			{
				if(m.name == name)
					return &m;
			}
			return nullptr;
		}
	};

	struct ast_node_enum_def_t : public ast_node_stmt_t
	{
		String name = "";
		std::map<String, i32_t> members = {};

		explicit ast_node_enum_def_t(ast_node_t* parent)
			: ast_node_stmt_t(ast_category_t::ENUM_DEF, parent)
		{ }
	};

	struct ast_node_proc_def_t : public ast_node_stmt_t
	{
		String name = "";
		ast_node_type_t* type = nullptr;
		std::map<String, ast_node_type_t*> args = {};

		explicit ast_node_proc_def_t(ast_node_t* parent)
			: ast_node_stmt_t(ast_category_t::PROC_DEF, parent)
		{ }
	};

	struct ast_node_return_t : public ast_node_stmt_t
	{
		ast_node_expr_t* value = nullptr;

		explicit ast_node_return_t(ast_node_t* parent)
			: ast_node_stmt_t(ast_category_t::RETURN, parent)
		{ }
	};

	struct ast_node_if_t : public ast_node_stmt_t
	{
		ast_node_expr_t* cond = nullptr;
		ast_node_stmt_t* branch_true = nullptr;
		ast_node_stmt_t* branch_false = nullptr;

		explicit ast_node_if_t(ast_node_t* parent)
			: ast_node_stmt_t(ast_category_t::IF, parent)
		{ }
	};

	struct ast_node_loop_t : public ast_node_stmt_t
	{
		ast_node_stmt_t* init = nullptr;
		ast_node_expr_t* cond = nullptr;
		ast_node_stmt_t* step = nullptr;
		ast_node_stmt_t* body = nullptr;

		explicit ast_node_loop_t(ast_node_t* parent)
			: ast_node_stmt_t(ast_category_t::LOOP, parent)
		{ }
	};

	struct ast_node_break_t : public ast_node_stmt_t
	{
		explicit ast_node_break_t(ast_node_t* parent)
			: ast_node_stmt_t(ast_category_t::BREAK, parent)
		{ }
	};

	struct ast_node_continue_t : public ast_node_stmt_t
	{
		explicit ast_node_continue_t(ast_node_t* parent)
			: ast_node_stmt_t(ast_category_t::CONTINUE, parent)
		{ }
	};

	struct ast_node_block_t : public ast_node_stmt_t
	{
		std::vector<ast_node_stmt_t*> stmts = {};

		explicit ast_node_block_t(ast_node_t* parent)
			: ast_node_stmt_t(ast_category_t::BLOCK, parent)
		{ }
	};

	struct ast_node_assign_t : public ast_node_stmt_t
	{
		ast_node_expr_t* left = nullptr;
		ast_node_expr_t* right = nullptr;

		explicit ast_node_assign_t(ast_node_t* parent)
			: ast_node_stmt_t(ast_category_t::ASSIGN, parent)
		{ }
	};
	
	struct ast_node_invoke_t : public ast_node_stmt_t
	{
		ast_node_func_ref_t* expr = nullptr;
		
		explicit ast_node_invoke_t(ast_node_t* parent)
			: ast_node_stmt_t(ast_category_t::INVOKE, parent)
		{ }
	};
}

#endif//_EOKAS_AST_NODES_H_
