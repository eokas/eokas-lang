#include "parser.h"
#include "scanner.h"
#include "../ast/ast.h"

namespace eokas
{
	parser_t::parser_t() 
		: scanner(new scanner_t())
		, factory(new ast_factory_t())
		, errormsg()
	{ }
	
	parser_t::~parser_t()
	{
		this->clear();
		_DeletePointer(this->scanner);
		_DeletePointer(this->factory);
	}
	
	ast_node_module_t* parser_t::parse(const char* source)
	{
		this->clear();
		this->scanner->ready(source);
		return this->parse_module();
	}
	
	void parser_t::clear()
	{
		this->scanner->clear();
		this->factory->clear();
		this->errormsg.clear();
	}
	
	ast_node_module_t* parser_t::parse_module()
	{
		if(!this->check_token(token_t::MODULE, true))
			return nullptr;
		
		auto* module = factory->create<ast_node_module_t>(nullptr);
		module->entry = factory->create<ast_node_func_def_t>(module);

		this->next_token();
		while (this->token().type != token_t::EOS)
		{
			// TODO: import
			// TODO: export
			ast_node_stmt_t* stmt = this->parse_stmt(module->entry);
			if(stmt == nullptr)
				return nullptr;
			module->entry->body.push_back(stmt);
		}
		
		return module;
	}
	
	ast_node_import_t* parser_t::parse_import(ast_node_t* p)
	{
		return nullptr;
	}
	
	ast_node_export_t* parser_t::parse_export(ast_node_t* p)
	{
		return nullptr;
	}
	
	/**
	 * type := ID '<' type '>'
	*/
	ast_node_type_t* parser_t::parse_type(ast_node_t* p)
	{
		if(!this->check_token(token_t::ID, true, false))
			return nullptr;
		
		String name = this->token().value;
		auto* node = factory->create<ast_node_type_t>(p);
		node->name = name;
		
		this->next_token(); // ignore ID
		
		// <
		if(this->check_token(token_t::LT, false))
		{
			// >
			while (!this->check_token(token_t::GT, false))
			{
				if(!node->args.empty() && !this->check_token(token_t::COMMA))
					return nullptr;
				
				auto* arg = this->parse_type(node);
				if(arg == nullptr)
					return nullptr;
				
				node->args.push_back(arg);
			}
			
			if(node->args.empty())
			{
				this->error("type arguments is empty.");
				return nullptr;
			}
		}
		
		return node;
	}
	
	ast_node_expr_t* parser_t::parse_expr(ast_node_t* p)
	{
		return this->parse_expr_trinary(p);
	}
	
	ast_node_expr_t* parser_t::parse_expr_trinary(ast_node_t* p)
	{
		ast_node_expr_t* binary = this->parse_expr_binary(p, 1);
		if(binary == nullptr)
			return nullptr;
		
		if(this->check_token(token_t::QUESTION, false))
		{
			auto* trinary = factory->create<ast_node_expr_trinary_t>(p);
			trinary->cond = binary;
			binary->parent = trinary;
			
			trinary->branch_true = this->parse_expr(trinary);
			if(trinary->branch_true == nullptr)
				return nullptr;
			
			if(!this->check_token(token_t::COLON))
				return nullptr;
			
			trinary->branch_false = this->parse_expr(trinary);
			if(trinary->branch_false == nullptr)
				return nullptr;
			
			return trinary;
		}
		
		return binary;
	}
	
	ast_node_expr_t* parser_t::parse_expr_binary(ast_node_t* p, int priority)
	{
		ast_node_expr_t* left = nullptr;
		
		if(priority<(int) ast_binary_oper_t::MAX_LEVEL / 100)
		{
			left = this->parse_expr_binary(p, priority + 1);
		}
		else
		{
			left = this->parse_expr_unary(p);
		}
		if(left == nullptr)
		{
			return nullptr;
		}
		
		for (;;)
		{
			ast_binary_oper_t oper = this->check_binary_oper(priority, false);
			if(oper == ast_binary_oper_t::UNKNOWN)
				break;
			
			
			ast_node_expr_t* right = nullptr;
			if(priority<(int) ast_binary_oper_t::MAX_LEVEL / 100)
			{
				right = this->parse_expr_binary(p, priority + 1);
			}
			else
			{
				right = this->parse_expr_unary(p);
			}
			if(right == nullptr)
			{
				_DeletePointer(left);
				return nullptr;
			}
			
			auto* binary = factory->create<ast_node_expr_binary_t>(p);
			binary->op = oper;
			binary->left = left;
			binary->right = right;
			
			left = binary;
		}
		
		return left;
	}
	
	/*
	expr_unary := expr_value | expr_construct | expr_suffixed
	expr_value := int | float | str | true | false
	*/
	ast_node_expr_t* parser_t::parse_expr_unary(ast_node_t* p)
	{
		ast_unary_oper_t oper = this->check_unary_oper(false, true);
		
		ast_node_expr_t* right = nullptr;
		
		token_t& token = this->token();
		switch (token.type)
		{
			case token_t::INT_B:
			case token_t::INT_X:
			case token_t::INT_D:
				right = this->parse_literal_int(p);
				break;
			case token_t::FLOAT:
				right = this->parse_literal_float(p);
				break;
			case token_t::STRING:
				right = this->parse_literal_string(p);
				break;
			case token_t::TRUE:
			case token_t::FALSE:
				right = this->parse_literal_bool(p);
				break;
			case token_t::FUNC:
				right = this->parse_func_def(p);
				break;
			case token_t::MAKE:
				right = this->parse_object_def(p);
				break;
				/*
				case token_t::Using:
					right = this->parse_module_ref(p);
					break;
				*/
			case token_t::LSB:
				right = this->parse_array_def(p);
				break;
			case token_t::ID:
			case token_t::LRB:
				right = this->parse_expr_suffixed(p);
				break;
			default:
				this->error_token_unexpected();
				return nullptr;
		}
		
		if(oper == ast_unary_oper_t::UNKNOWN)
			return right;
		
		auto* unary = factory->create<ast_node_expr_unary_t>(p);
		unary->op = oper;
		unary->right = right;
		right->parent = unary;
		
		return unary;
	}
	
	/*
	expr_suffixed := expr_primary{ '.'ID | ':'ID | '['expr']' | '{'stat_list'}' | proc_args }
	*/
	ast_node_expr_t* parser_t::parse_expr_suffixed(ast_node_t* p)
	{
		ast_node_expr_t* primary = this->parse_expr_primary(p);
		if(primary == nullptr)
			return nullptr;
		
		for (;;)
		{
			ast_node_expr_t* suffixed = nullptr;
			
			token_t& token = this->token();
			switch (token.type)
			{
				case token_t::DOT: // .
					suffixed = this->parse_object_ref(p, primary);
					break;
				case token_t::LSB: // [
					suffixed = this->parse_index_ref(p, primary);
					break;
				case token_t::LRB: // (
					suffixed = this->parse_func_call(p, primary);
					break;
				default: // no more invalid suffix
					return primary;
			}
			
			if(suffixed == nullptr)
				return nullptr;
			
			primary = suffixed;
		}
		
		return primary;
	}
	
	/*
	expr_primary := ID | '(' expr ')'
	*/
	ast_node_expr_t* parser_t::parse_expr_primary(ast_node_t* p)
	{
		if(this->check_token(token_t::LRB, false))
		{
			ast_node_expr_t* expr = this->parse_expr(p);
			if(expr == nullptr)
				return nullptr;
			
			if(!this->check_token(token_t::RRB))
				return nullptr;
			
			return expr;
		}
		
		if(!this->check_token(token_t::ID, true, false))
			return nullptr;
		
		auto* node = factory->create<ast_node_symbol_ref_t>(p);
		node->name = this->token().value;
		
		this->next_token();
		
		return node;
	}
	
	ast_node_expr_t* parser_t::parse_literal_int(ast_node_t* p)
	{
		token_t& token = this->token();
		switch (token.type)
		{
			case token_t::INT_B:
			{
				auto* node = factory->create<ast_node_literal_int_t>(p);
				node->value = String::binstrToValue<i32_t>(token.value);
				this->next_token();
				return node;
			}
			case token_t::INT_X:
			{
				auto* node = factory->create<ast_node_literal_int_t>(p);
				node->value = String::hexstrToValue<i32_t>(token.value);
				this->next_token();
				return node;
			}
			case token_t::INT_D:
			{
				auto* node = factory->create<ast_node_literal_int_t>(p);
				node->value = String::stringToValue<i32_t>(token.value);
				this->next_token();
				return node;
			}
			default:
				this->error_token_unexpected();
				return nullptr;
		}
	}
	
	ast_node_expr_t* parser_t::parse_literal_float(ast_node_t* p)
	{
		token_t& token = this->token();
		switch (token.type)
		{
			case token_t::FLOAT:
			{
				auto* node = factory->create<ast_node_literal_float_t>(p);
				node->value = String::stringToValue<f32_t>(token.value);
				this->next_token();
				return node;
			}
			default:
				this->error_token_unexpected();
				return nullptr;
		}
	}
	
	ast_node_expr_t* parser_t::parse_literal_bool(ast_node_t* p)
	{
		token_t& token = this->token();
		switch (token.type)
		{
			case token_t::TRUE:
			case token_t::FALSE:
			{
				auto* node = factory->create<ast_node_literal_bool_t>(p);
				node->value = String::stringToValue<bool>(token.value);
				this->next_token();
				return node;
			}
			default:
				this->error_token_unexpected();
				return nullptr;
		}
	}
	
	ast_node_expr_t* parser_t::parse_literal_string(ast_node_t* p)
	{
		token_t& token = this->token();
		switch (token.type)
		{
			case token_t::STRING:
			{
				auto* node = factory->create<ast_node_literal_string_t>(p);
				node->value = token.value;
				this->next_token();
				return node;
			}
			default:
				this->error_token_unexpected();
				return nullptr;
		}
	}
	
	/*
	func_def => 'func' func_params func_body
	*/
	ast_node_expr_t* parser_t::parse_func_def(ast_node_t* p)
	{
		if(!this->check_token(token_t::FUNC))
			return nullptr;
		
		auto* node = factory->create<ast_node_func_def_t>(p);
		if(!this->parse_func_params(node))
			return nullptr;
		
		// : ret-type
		if(!this->check_token(token_t::COLON))
			return nullptr;
		
		node->rtype = this->parse_type(node);
		if(node->rtype == nullptr)
			return nullptr;
		
		if(!this->parse_func_body(node))
			return nullptr;
		
		return node;
	}
	
	/*
	func_params => '(' [ID] ')'
	*/
	bool parser_t::parse_func_params(ast_node_func_def_t* node)
	{
		if(!this->check_token(token_t::LRB)) // (
			return false;
		
		do
		{
			if(this->token().type == token_t::RRB)
				break;
			
			if(!this->check_token(token_t::ID, true, false))
				return false;
			const String name = this->token().value;
			if(node->getArg(name) != nullptr)
			{
				this->error_token_unexpected();
				return false;
			}
			this->next_token();
			
			if(!this->check_token(token_t::COLON))
				return false;
			
			ast_node_type_t* type = this->parse_type(node);
			if(type == nullptr)
				return false;
			
			auto* arg = node->addArg(name);
			if(arg == nullptr)
			{
				this->error_token_unexpected();
				return false;
			}
			
			arg->name = name;
			arg->type = type;
		}
		while (this->check_token(token_t::COMMA, false));
		
		if(!this->check_token(token_t::RRB))
			return false;
		
		return true;
	}
	
	/*
	func_body => '{' [stat] '}'
	*/
	bool parser_t::parse_func_body(ast_node_func_def_t* node)
	{
		if(!this->check_token(token_t::LCB))
			return false;
		
		while (!this->check_token(token_t::RCB, false))
		{
			ast_node_stmt_t* stmt = this->parse_stmt(node);
			if(stmt == nullptr)
				return false;
			node->body.push_back(stmt);
		}
		
		return true;
	}
	
	/*
	func_call => '(' expr, expr, ..., expr ')'
	*/
	ast_node_expr_t* parser_t::parse_func_call(ast_node_t* p, ast_node_expr_t* primary)
	{
		auto* node = factory->create<ast_node_func_ref_t>(p);
		
		if(!this->check_token(token_t::LRB))
			return nullptr;
		
		while (!this->check_token(token_t::RRB, false))
		{
			if(!node->args.empty() && !this->check_token(token_t::COMMA))
				return nullptr;
			
			ast_node_expr_t* arg = this->parse_expr(node);
			if(arg == nullptr)
				return nullptr;
			
			node->args.push_back(arg);
		}
		
		// 确保所有解析成功后，才能将 primary 赋值给 node，
		// 否则会出现 crash。
		node->func = primary;
		primary->parent = node;
		
		return node;
	}
	
	/*
	object_def => 'make' type_ref '{' [object_field {sep object_field} [sep]] '}'
	sep => ',' | ';'
	*/
	ast_node_expr_t* parser_t::parse_object_def(ast_node_t* p)
	{
		if(!this->check_token(token_t::MAKE))
			return nullptr;
		
		auto* node = factory->create<ast_node_object_def_t>(p);
		node->type = this->parse_type(node);
		if(node->type == nullptr)
			return nullptr;
		
		if(!this->check_token(token_t::LCB))
			return nullptr;
		
		do
		{
			if(this->token().type == token_t::RCB)
				break;
			if(!this->parse_object_field(node))
				return nullptr;
		} while (this->check_token(token_t::COMMA, false));
		
		if(!this->check_token(token_t::RCB))
			return nullptr;
		
		return node;
	}
	
	/*
	object_field => ID '=' expr
	*/
	bool parser_t::parse_object_field(ast_node_object_def_t* node)
	{
		if(!this->check_token(token_t::ID, true, false))
			return false;
		
		const String key = this->token().value;
		this->next_token();
		
		if(!this->check_token(token_t::ASSIGN, false) && !this->check_token(token_t::COLON, false))
		{
			return false;
		}
		
		ast_node_expr_t* expr = this->parse_expr(node);
		if(expr == nullptr)
			return false;
		
		node->members[key] = expr;
		
		return true;
	}
	
	/*
	array_def => '[' [array_field {sep array_field} [sep]] ']'
	sep => ',' | ';'
	*/
	ast_node_expr_t* parser_t::parse_array_def(ast_node_t* p)
	{
		if(!this->check_token(token_t::LSB))
			return nullptr;
		
		auto* node = factory->create<ast_node_array_def_t>(p);
		
		do
		{
			if(this->token().type == token_t::RSB)
				break;
			
			ast_node_expr_t* expr = this->parse_expr(node);
			if(expr == nullptr)
				return nullptr;
			
			node->elements.push_back(expr);
		} while (this->check_token(token_t::COMMA, false));
		
		if(!this->check_token(token_t::RSB))
			return nullptr;
		
		return node;
	}
	
	/*
	index_ref => '[' expr ']'
	*/
	ast_node_expr_t* parser_t::parse_index_ref(ast_node_t* p, ast_node_expr_t* primary)
	{
		auto* node = factory->create<ast_node_array_ref_t>(p);
		
		if(!this->check_token(token_t::LSB))
			return nullptr;
		
		node->key = this->parse_expr(node);
		if(node->key == nullptr)
			return nullptr;
		
		if(!this->check_token(token_t::RSB))
			return nullptr;
		
		node->obj = primary;
		primary->parent = node;
		
		return node;
	}
	
	/*
	object_ref => '.' ID
	*/
	ast_node_expr_t* parser_t::parse_object_ref(ast_node_t* p, ast_node_expr_t* primary)
	{
		auto* node = factory->create<ast_node_object_ref_t>(p);
		
		if(!this->check_token(token_t::DOT))
			return nullptr;
		
		if(!this->check_token(token_t::ID, true, false))
			return nullptr;
		
		node->key = this->token().value;
		
		this->next_token();
		
		node->obj = primary;
		primary->parent = node;
		
		return node;
	}
	
	ast_node_stmt_t* parser_t::parse_stmt(ast_node_t* p)
	{
		ast_node_stmt_t* stmt = nullptr;
		bool semicolon = false;
		
		switch (this->token().type)
		{
			case token_t::STRUCT:
				stmt = this->parse_stmt_struct_def(p);
				break;
			case token_t::ENUM:
				stmt = this->parse_stmt_enum_def(p);
				break;
			case token_t::PROC:
				stmt = this->parse_stmt_proc_def(p);
				semicolon = true;
				break;
			case token_t::VAR:
			case token_t::VAL:
				stmt = this->parse_stmt_symbol_def(p);
				semicolon = true;
				break;
			case token_t::BREAK:
				stmt = this->parse_stmt_break(p);
				semicolon = true;
				break;
			case token_t::CONTINUE:
				stmt = this->parse_stmt_continue(p);
				semicolon = true;
				break;
			case token_t::RETURN:
				stmt = this->parse_stmt_return(p);
				semicolon = true;
				break;
			case token_t::IF:
				stmt = this->parse_stmt_if(p);
				break;
			case token_t::LOOP:
				stmt = this->parse_stmt_loop(p);
				break;
			case token_t::LCB:
				stmt = this->parse_stmt_block(p);
				break;
			default:
				stmt = this->parse_stmt_assign_or_call(p);
				semicolon = true;
		}
		
		if(semicolon && !this->check_token(token_t::SEMICOLON))
			return nullptr;
		
		return stmt;
	}
	
	/**
	 * struct_def := 'struct' ID '{' struct_member '};';
	*/
	ast_node_struct_def_t* parser_t::parse_stmt_struct_def(ast_node_t* p)
	{
		if(!this->check_token(token_t::STRUCT))
			return nullptr;
		
		auto* node = factory->create<ast_node_struct_def_t>(p);
		
		// ID
		if(!this->check_token(token_t::ID, true, false))
			return nullptr;
		
		node->name = this->token().value;
		this->next_token();
		
		// {
		if(!this->check_token(token_t::LCB))
			return nullptr;
		
		do
		{
			if(this->token().type == token_t::RCB)
				break;
			
			if(!this->parse_stmt_struct_member(node))
				return nullptr;
		}
		while (this->check_token(token_t::SEMICOLON, false));
		
		// }
		if(!this->check_token(token_t::RCB))
			return nullptr;
		
		return node;
	}
	
	/**
	 * struct_member := ('var' | 'val') ID : type [ '=' expr ] ;
	*/
	bool parser_t::parse_stmt_struct_member(ast_node_struct_def_t* p)
	{
		// (val | var)
		bool isConst = false;
		switch (this->token().type)
		{
			case token_t::VAL: isConst = true;
			case token_t::VAR: isConst = false;
			default:
				return false;
		}
		this->next_token();
		
		// ID
		if(!this->check_token(token_t::ID, true, false))
			return false;
		
		const String& name = this->token().value;
		auto* node = p->addMember(name);
		if(node == nullptr)
		{
			this->error_token_unexpected();
			return false;
		}
		this->next_token();
		
		// : type
		if(!this->check_token(token_t::COLON))
			return false;
		node->type = this->parse_type(p);
		if(node->type == nullptr)
			return false;
		
		// [= expr]
		if(this->check_token(token_t::ASSIGN, false))
		{
			node->value = this->parse_expr(p);
			if(node->value == nullptr)
				return false;
		}
		
		return true;
	}
	
	/**
	 * enum_def := 'enum' ID '{' [enum_member] '}' ';';
	 * enum_member := ID ['=' expr_int] ',';
	 * */
	ast_node_enum_def_t* parser_t::parse_stmt_enum_def(ast_node_t* p)
	{
		if(!this->check_token(token_t::ENUM))
			return nullptr;
		
		auto* node = factory->create<ast_node_enum_def_t>(p);
		
		// ID
		if(!this->check_token(token_t::ID, true, false))
			return nullptr;
		
		node->name = this->token().value;
		this->next_token();
		
		// {
		if(!this->check_token(token_t::LCB))
			return nullptr;
		
		int index = 0;
		do
		{
			if(this->token().type == token_t::RCB)
				break;
			
			// ID
			if(!this->check_token(token_t::ID, true, false))
				return nullptr;
			
			const String memName = this->token().value;
			if(node->members.find(memName) != node->members.end())
			{
				this->error_token_unexpected();
				return nullptr;
			}
			this->next_token();
			
			// [= int]
			i32_t memValue = index;
			if(this->check_token(token_t::ASSIGN, false))
			{
				auto memExpr = this->parse_literal_int(node);
				if(memExpr == nullptr)
					return nullptr;
				auto memIntExpr = dynamic_cast<ast_node_literal_int_t*>(memExpr);
				memValue = static_cast<i32_t>(memIntExpr->value);
				index = memValue;
			}
			
			node->members[memName] = memValue;
			index += 1;
		} while (this->check_token(token_t::COMMA, false));
		
		// }
		if(!this->check_token(token_t::RCB))
			return nullptr;
		
		return node;
	}
	
	/**
	 * proc_def := 'proc' ID '(' [func_params]* ')' ';';
	 * func_params := ID ':' type_ref ','
	*/
	ast_node_proc_def_t* parser_t::parse_stmt_proc_def(ast_node_t* p)
	{
		if(!this->check_token(token_t::PROC))
			return nullptr;
		
		auto* node = factory->create<ast_node_proc_def_t>(p);
		
		// ID
		if(!this->check_token(token_t::ID, true, false))
			return nullptr;
		
		node->name = this->token().value;
		this->next_token();
		
		// (
		if(!this->check_token(token_t::LRB))
			return nullptr;
		
		do
		{
			if(this->token().type == token_t::RRB)
				break;
			
			// ID
			if(!this->check_token(token_t::ID, true, false))
				return nullptr;
			
			const String argName = this->token().value;
			if(node->args.find(argName) != node->args.end())
			{
				this->error_token_unexpected();
				return nullptr;
			}
			this->next_token();
			
			// : type
			if(!this->check_token(token_t::COLON))
				return nullptr;
			ast_node_type_t* argType = this->parse_type(node);
			if(argType == nullptr)
				return nullptr;
			
			node->args[argName] = argType;
		} while (this->check_token(token_t::COMMA, false));
		
		// )
		if(!this->check_token(token_t::RRB))
			return nullptr;
		
		// : ret-type
		if(!this->check_token(token_t::COLON))
			return nullptr;
		node->type = this->parse_type(node);
		if(node->type == nullptr)
			return nullptr;
		
		return node;
	}
	
	/**
	 * symbol_def := 'var' | 'val' ID [: type] = expr;
	*/
	ast_node_symbol_def_t* parser_t::parse_stmt_symbol_def(ast_node_t* p)
	{
		if(!this->check_token(token_t::VAR, false, false) && !this->check_token(token_t::VAL, false, false))
		{
			return nullptr;
		}
		
		auto* node = factory->create<ast_node_symbol_def_t>(p);
		node->variable = this->token().type == token_t::VAR;
		
		this->next_token(); // ignore 'var' | 'val'
		
		if(!this->check_token(token_t::ID, true, false))
			return nullptr;
		
		node->name = this->token().value;
		
		this->next_token();
		
		// : type
		if(this->check_token(token_t::COLON, false))
		{
			node->type = this->parse_type(p);
			if(node->type == nullptr)
				return nullptr;
		}
		
		// = expr
		if(!this->check_token(token_t::ASSIGN, true))
			return nullptr;
		
		node->value = this->parse_expr(node);
		if(node->value == nullptr)
			return nullptr;
		
		return node;
	}
	
	ast_node_continue_t* parser_t::parse_stmt_continue(ast_node_t* p)
	{
		if(!this->check_token(token_t::CONTINUE))
			return nullptr;
		
		auto* node = factory->create<ast_node_continue_t>(p);
		
		return node;
	}
	
	ast_node_break_t* parser_t::parse_stmt_break(ast_node_t* p)
	{
		if(!this->check_token(token_t::BREAK))
			return nullptr;
		
		auto* node = factory->create<ast_node_break_t>(p);
		
		return node;
	}
	
	ast_node_return_t* parser_t::parse_stmt_return(ast_node_t* p)
	{
		if(!this->check_token(token_t::RETURN))
			return nullptr;
		
		auto* node = factory->create<ast_node_return_t>(p);
		
		if(this->check_token(token_t::SEMICOLON, false, false))
			return node;
		
		node->value = this->parse_expr(node);
		if(node->value == nullptr)
			return nullptr;
		
		return node;
	}
	
	ast_node_if_t* parser_t::parse_stmt_if(ast_node_t* p)
	{
		if(!this->check_token(token_t::IF))
			return nullptr;
		
		auto* node = factory->create<ast_node_if_t>(p);
		
		if(!this->check_token(token_t::LRB))
			return nullptr;
		
		node->cond = this->parse_expr(node);
		if(node->cond == nullptr)
			return nullptr;
		
		if(!this->check_token(token_t::RRB))
			return nullptr;
		
		node->branch_true = this->parse_stmt(node);
		if(node->branch_true == nullptr)
			return nullptr;
		
		if(this->check_token(token_t::ELSE, false))
		{
			node->branch_false = this->parse_stmt(node);
			if(node->branch_false == nullptr)
				return nullptr;
		}
		
		return node;
	}
	
	ast_node_loop_t* parser_t::parse_stmt_loop(ast_node_t* p)
	{
		if(!this->check_token(token_t::LOOP))
			return nullptr;
		
		auto* node = factory->create<ast_node_loop_t>(p);
		
		if(!this->check_token(token_t::LRB))
			return nullptr;
		
		node->init = this->parse_stmt_loop_init(node);
		if(node->init == nullptr)
			return nullptr;
		
		node->cond = this->parse_stmt_loop_cond(node);
		if(node->cond == nullptr)
			return nullptr;
		
		node->step = this->parse_stmt_loop_step(node);
		if(node->step == nullptr)
			return nullptr;
		
		if(!this->check_token(token_t::RRB))
			return nullptr;
		
		node->body = this->parse_stmt(node);
		if(node->body == nullptr)
			return nullptr;
		
		return node;
	}
	
	ast_node_stmt_t* parser_t::parse_stmt_loop_init(ast_node_t* p)
	{
		auto* stmt = this->parse_stmt_symbol_def(p);
		if(stmt == nullptr)
			return nullptr;
		
		if(!this->check_token(token_t::SEMICOLON))
			return nullptr;
		
		return stmt;
	}
	
	ast_node_expr_t* parser_t::parse_stmt_loop_cond(ast_node_t* p)
	{
		ast_node_expr_t* cond = this->parse_expr(p);
		if(cond == nullptr)
			return nullptr;
		
		if(!this->check_token(token_t::SEMICOLON))
			return nullptr;
		
		return cond;
	}
	
	ast_node_stmt_t* parser_t::parse_stmt_loop_step(ast_node_t* p)
	{
		ast_node_stmt_t* stmt = nullptr;
		switch (this->token().type)
		{
			case token_t::VAR:
			case token_t::VAL:
				stmt = this->parse_stmt_symbol_def(p);
				break;
			default:
				stmt = this->parse_stmt_assign_or_call(p);
				break;
		}
		
		return stmt;
	}
	
	ast_node_block_t* parser_t::parse_stmt_block(ast_node_t* p)
	{
		if(!this->check_token(token_t::LCB))
			return nullptr;
		
		auto* node = factory->create<ast_node_block_t>(p);
		
		while (!this->check_token(token_t::RCB, false))
		{
			auto* stmt = this->parse_stmt(node);
			if(stmt == nullptr)
				return nullptr;
			
			node->stmts.push_back(stmt);
			
			this->check_token(token_t::SEMICOLON, false);
		}
		
		return node;
	}
	
	ast_node_stmt_t* parser_t::parse_stmt_assign_or_call(ast_node_t* p)
	{
		ast_node_expr_t* left = this->parse_expr_suffixed(p);
		if(left == nullptr)
			return nullptr;
		
		if(this->check_token(token_t::ASSIGN, false))
		{
			auto* node = factory->create<ast_node_assign_t>(p);
			node->left = left;
			left->parent = node;
			
			node->right = this->parse_expr(node);
			if(node->right == nullptr)
				return nullptr;
			
			return node;
		}
		else if(left->category == ast_category_t::FUNC_REF)
		{
			auto* node = factory->create<ast_node_invoke_t>(p);
			node->expr = dynamic_cast<ast_node_func_ref_t*>(left);
			left->parent = node;
			
			return node;
		}
		else
		{
			this->error_token_unexpected();
			return nullptr;
		}
	}
	
	void parser_t::next_token()
	{
		return scanner->next_token();
	}
	
	token_t& parser_t::token()
	{
		return scanner->token();
	}
	
	token_t& parser_t::look_ahead_token()
	{
		return scanner->look_ahead_token();
	}
	
	bool parser_t::check_token(const token_t::token_type& tokenType, bool required, bool movenext)
	{
		if(scanner->token().type != tokenType)
		{
			if(required)
			{
				this->error_token_unexpected();
			}
			return false;
		}
		if(movenext)
		{
			scanner->next_token();
		}
		return true;
	}
	
	ast_unary_oper_t parser_t::check_unary_oper(bool required, bool movenext)
	{
		ast_unary_oper_t oper = ast_unary_oper_t::UNKNOWN;
		switch (scanner->token().type)
		{
			case token_t::ADD:
				oper = ast_unary_oper_t::POS;
				break;
			case token_t::SUB:
				oper = ast_unary_oper_t::NEG;
				break;
			case token_t::NOT:
				oper = ast_unary_oper_t::NOT;
				break;
			case token_t::FLIP:
				oper = ast_unary_oper_t::FLIP;
				break;
			case token_t::AT:
				oper = ast_unary_oper_t::TYPE_OF;
				break;
			case token_t::POUND:
				oper = ast_unary_oper_t::SIZE_OF;
				break;
			default:
				break;
		}
		if(oper == ast_unary_oper_t::UNKNOWN)
		{
			if(required)
			{
				this->error_token_unexpected();
			}
			return oper;
		}
		if(movenext)
		{
			scanner->next_token();
		}
		return oper;
	}
	
	ast_binary_oper_t parser_t::check_binary_oper(int priority, bool required, bool movenext)
	{
		ast_binary_oper_t oper = ast_binary_oper_t::UNKNOWN;
		switch (scanner->token().type)
		{
			case token_t::OR2:
				oper = ast_binary_oper_t::OR;
				break;
			case token_t::AND2:
				oper = ast_binary_oper_t::AND;
				break;
			case token_t::EQ:
				oper = ast_binary_oper_t::EQ;
				break;
			case token_t::GT:
				oper = ast_binary_oper_t::GT;
				break;
			case token_t::LT:
				oper = ast_binary_oper_t::LT;
				break;
			case token_t::GE:
				oper = ast_binary_oper_t::GE;
				break;
			case token_t::LE:
				oper = ast_binary_oper_t::LE;
				break;
			case token_t::NE:
				oper = ast_binary_oper_t::NE;
				break;
			case token_t::ADD:
				oper = ast_binary_oper_t::ADD;
				break;
			case token_t::SUB:
				oper = ast_binary_oper_t::SUB;
				break;
			case token_t::MUL:
				oper = ast_binary_oper_t::MUL;
				break;
			case token_t::DIV:
				oper = ast_binary_oper_t::DIV;
				break;
			case token_t::MOD:
				oper = ast_binary_oper_t::MOD;
				break;
			case token_t::AND:
				oper = ast_binary_oper_t::BIT_AND;
				break;
			case token_t::OR:
				oper = ast_binary_oper_t::BIT_OR;
				break;
			case token_t::XOR:
				oper = ast_binary_oper_t::BIT_XOR;
				break;
			case token_t::SHIFT_L:
				oper = ast_binary_oper_t::SHIFT_L;
				break;
			case token_t::SHIFT_R:
				oper = ast_binary_oper_t::SHIFT_R;
				break;
			default:
				break;
		}
		if((int) oper / 100 != priority)
		{
			oper = ast_binary_oper_t::UNKNOWN;
		}
		if(oper == ast_binary_oper_t::UNKNOWN)
		{
			if(required)
			{
				this->error_token_unexpected();
			}
			return oper;
		}
		if(movenext)
		{
			scanner->next_token();
		}
		return oper;
	}
	
	void parser_t::error(const char* fmt, ...)
	{
		String message;
		_FormatVA(message, fmt);
		errormsg = String::format("%s at %d, %d.\n", message.cstr(), scanner->line(), scanner->column());
	}
	
	void parser_t::error_token_unexpected()
	{
		token_t& token = scanner->token();
		const char* value = token.value.cstr();
		if(token.type == token_t::EOS)
		{
			this->error("unexpected eos");
		}
		else
		{
			this->error("unexpected token '%s'", value);
		}
	}
	
	const String& parser_t::error() const
	{
		return errormsg;
	}
}
