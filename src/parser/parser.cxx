#include "parser.h"
#include "scanner.h"
#include "../ast/ast.h"

_BeginNamespace(eokas)
	
	class parser_impl_t
	{
	public:
		parser_impl_t();
		
		virtual ~parser_impl_t();
	
	public:
		void clear();
		
		ast_module_t* parse_module(const char* source);
		ast_type_t* parse_type(ast_node_t* p);
		ast_type_ref_t* parse_type_ref(ast_node_t* p);
		ast_type_array_t* parse_type_array(ast_node_t* p);
		ast_type_generic_t* parse_type_generic(ast_node_t* p);
		ast_expr_t* parse_expr(ast_node_t* p);
		ast_expr_t* parse_expr_trinary(ast_node_t* p);
		ast_expr_t* parse_expr_binary(ast_node_t* p, int priority = 1);
		ast_expr_t* parse_expr_unary(ast_node_t* p);
		ast_expr_t* parse_expr_suffixed(ast_node_t* p);
		ast_expr_t* parse_expr_primary(ast_node_t* p);
		ast_expr_t* parse_literal_int(ast_node_t* p);
		ast_expr_t* parse_literal_float(ast_node_t* p);
		ast_expr_t* parse_literal_bool(ast_node_t* p);
		ast_expr_t* parse_literal_string(ast_node_t* p);
		ast_expr_t* parse_func_def(ast_node_t* p);
		bool parse_func_params(ast_expr_func_def_t* node);
		bool parse_func_body(ast_expr_func_def_t* node);
		ast_expr_t* parse_object_def(ast_node_t* p);
		ast_expr_t* parse_func_call(ast_node_t* p, ast_expr_t* primary);
		ast_expr_t* parse_array_def(ast_node_t* p);
		ast_expr_t* parse_index_ref(ast_node_t* p, ast_expr_t* primary);
		bool parse_object_field(ast_expr_object_def_t* node);
		ast_expr_t* parse_object_ref(ast_node_t* p, ast_expr_t* primary);
		ast_expr_t* parse_module_ref(ast_node_t* p);
		ast_stmt_t* parse_stmt(ast_node_t* p);
		ast_stmt_struct_def_t* parse_stmt_struct_def(ast_node_t* p);
		ast_stmt_struct_member_t* parse_stmt_struct_member(ast_node_t* p);
		ast_stmt_enum_def_t* parse_stmt_enum_def(ast_node_t* p);
		ast_stmt_proc_def_t* parse_stmt_proc_def(ast_node_t* p);
		ast_stmt_symbol_def_t* parse_stmt_symbol_def(ast_node_t* p);
		ast_stmt_continue_t* parse_stmt_continue(ast_node_t* p);
		ast_stmt_break_t* parse_stmt_break(ast_node_t* p);
		ast_stmt_return_t* parse_stmt_return(ast_node_t* p);
		ast_stmt_if_t* parse_stmt_if(ast_node_t* p);
		ast_stmt_loop_t* parse_stmt_loop(ast_node_t* p);
		ast_stmt_t* parse_stmt_loop_init(ast_node_t* p);
		ast_expr_t* parse_stmt_loop_cond(ast_node_t* p);
		ast_stmt_t* parse_stmt_loop_step(ast_node_t* p);
		ast_stmt_block_t* parse_stmt_block(ast_node_t* p);
		ast_stmt_t* parse_stmt_assign_or_call(ast_node_t* p);
		
		void next_token();
		struct token_t& token();
		struct token_t& look_ahead_token();
		bool check_token(const token_t::token_type& tokenType, bool required = true, bool movenext = true);
		ast_unary_oper_t check_unary_oper(bool required = true, bool movenext = true);
		ast_binary_oper_t check_binary_oper(int priority, bool required = true, bool movenext = true);
		const String& error();
	
	private:
		void error(const char* fmt, ...);
		
		void error_token_unexpected();
	
	private:
		class scanner_t* scanner;
		class ast_factory_t* factory;
		String errormsg;
	};
	
	
	parser_impl_t::parser_impl_t() : scanner(new scanner_t()), factory(nullptr), errormsg()
	{
	}
	
	parser_impl_t::~parser_impl_t()
	{
		this->clear();
		_DeletePointer(this->scanner);
	}
	
	void parser_impl_t::clear()
	{
		this->errormsg.clear();
		this->scanner->clear();
	}
	
	ast_module_t* parser_impl_t::parse_module(const char* source)
	{
		this->clear();
		this->scanner->ready(source);
		
		auto* module = new ast_module_t();
		this->factory = module->get_factory();
		
		auto* entry = module->get_func();
		
		this->next_token();
		while (this->token().type != token_t::Eos)
		{
			ast_stmt_t* stmt = this->parse_stmt(entry);
			if(stmt == nullptr)
				return nullptr;
			entry->body.push_back(stmt);
		}
		
		return module;
	}
	
	ast_type_t* parser_impl_t::parse_type(ast_node_t* p)
	{
		if(this->token().type == token_t::Array)
		{
			return this->parse_type_array(p);
		}
		else if(this->token().type == token_t::ID)
		{
			if(this->look_ahead_token().type == token_t::Less)
			{
				return this->parse_type_generic(p);
			}
			else
			{
				return this->parse_type_ref(p);
			}
		}
		else
		{
			this->error_token_unexpected();
			return nullptr;
		}
	}
	
	/**
	 * type_ref := ID
	*/
	ast_type_ref_t* parser_impl_t::parse_type_ref(ast_node_t* p)
	{
		if(!this->check_token(token_t::ID, true, false))
			return nullptr;
		
		String name = this->token().value;
		auto* node = factory->create_type_ref(p);
		node->name = name;
		
		this->next_token(); // ignore ID
		
		return node;
	}
	
	/**
	 * type_array := 'array' '<' type ',' int_num '>';
	*/
	ast_type_array_t* parser_impl_t::parse_type_array(ast_node_t* p)
	{
		if(!this->check_token(token_t::Array))
			return nullptr;
		
		if(!this->check_token(token_t::Less))
			return nullptr;
		
		auto* node = factory->create_type_array(p);
		node->elementType = this->parse_type(node);
		if(node->elementType == nullptr)
			return nullptr;
		
		if(!this->check_token(token_t::Comma))
			return nullptr;
		
		if(!this->check_token(token_t::DInt, true, false))
			return nullptr;
		
		node->length = String::stringToValue<i32_t>(this->token().value);
		this->next_token(); // ignore length.
		
		if(!this->check_token(token_t::Greater))
			return nullptr;
		
		return node;
	}
	
	/**
	 * type_generic := ID '<' type '>'
	*/
	ast_type_generic_t* parser_impl_t::parse_type_generic(ast_node_t* p)
	{
		if(!this->check_token(token_t::ID, true, false))
			return nullptr;
		
		auto* node = factory->create_type_generic(p);
		node->name = this->token().value;
		this->next_token(); // ignore ID
		
		// <
		if(this->check_token(token_t::Less, false))
		{
			// >
			while (!this->check_token(token_t::Greater, false))
			{
				if(!node->args.empty() && !this->check_token(token_t::Comma))
					return nullptr;
				
				ast_type_ref_t* arg = this->parse_type_ref(node);
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
	
	ast_expr_t* parser_impl_t::parse_expr(ast_node_t* p)
	{
		return this->parse_expr_trinary(p);
	}
	
	ast_expr_t* parser_impl_t::parse_expr_trinary(ast_node_t* p)
	{
		ast_expr_t* binary = this->parse_expr_binary(p, 1);
		if(binary == nullptr)
			return nullptr;
		
		if(this->check_token(token_t::Question, false))
		{
			auto* trinary = factory->create_expr_trinary(p);
			trinary->cond = binary;
			binary->parent = trinary;
			
			trinary->branch_true = this->parse_expr(trinary);
			if(trinary->branch_true == nullptr)
				return nullptr;
			
			if(!this->check_token(token_t::Colon))
				return nullptr;
			
			trinary->branch_false = this->parse_expr(trinary);
			if(trinary->branch_false == nullptr)
				return nullptr;
			
			return trinary;
		}
		
		return binary;
	}
	
	ast_expr_t* parser_impl_t::parse_expr_binary(ast_node_t* p, int priority)
	{
		ast_expr_t* left = nullptr;
		
		if(priority<(int) ast_binary_oper_t::MaxPriority / 100)
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
			if(oper == ast_binary_oper_t::Unknown)
				break;
			
			if(oper == ast_binary_oper_t::Is || oper == ast_binary_oper_t::As)
			{
				ast_type_t* right = right = this->parse_type(p);
				if(right == nullptr)
				{
					_DeletePointer(left);
					return nullptr;
				}
				
				auto* binary = factory->create_expr_binary_type(p);
				binary->op = oper;
				binary->left = left;
				binary->right = right;
				
				left = binary;
			}
			else
			{
				ast_expr_t* right = nullptr;
				if(priority<(int) ast_binary_oper_t::MaxPriority / 100)
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
				
				auto* binary = factory->create_expr_binary_value(p);
				binary->op = oper;
				binary->left = left;
				binary->right = right;
				
				left = binary;
			}
		}
		
		return left;
	}
	
	/*
	expr_unary := expr_value | expr_construct | expr_suffixed
	expr_value := int | float | str | true | false
	*/
	ast_expr_t* parser_impl_t::parse_expr_unary(ast_node_t* p)
	{
		ast_unary_oper_t oper = this->check_unary_oper(false, true);
		
		ast_expr_t* right = nullptr;
		
		token_t& token = this->token();
		switch (token.type)
		{
			case token_t::BInt:
			case token_t::XInt:
			case token_t::DInt:
				right = this->parse_literal_int(p);
				break;
			case token_t::Float:
				right = this->parse_literal_float(p);
				break;
			case token_t::Str:
				right = this->parse_literal_string(p);
				break;
			case token_t::True:
			case token_t::False:
				right = this->parse_literal_bool(p);
				break;
			case token_t::Func:
				right = this->parse_func_def(p);
				break;
			case token_t::Make:
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
		
		if(oper == ast_unary_oper_t::Unknown)
			return right;
		
		auto* unary = factory->create_expr_unary(p);
		unary->op = oper;
		unary->right = right;
		right->parent = unary;
		
		return unary;
	}
	
	/*
	expr_suffixed := expr_primary{ '.'ID | ':'ID | '['expr']' | '{'stat_list'}' | proc_args }
	*/
	ast_expr_t* parser_impl_t::parse_expr_suffixed(ast_node_t* p)
	{
		ast_expr_t* primary = this->parse_expr_primary(p);
		if(primary == nullptr)
			return nullptr;
		
		for (;;)
		{
			ast_expr_t* suffixed = nullptr;
			
			token_t& token = this->token();
			switch (token.type)
			{
				case token_t::Dot: // .
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
	ast_expr_t* parser_impl_t::parse_expr_primary(ast_node_t* p)
	{
		if(this->check_token(token_t::LRB, false))
		{
			ast_expr_t* expr = this->parse_expr(p);
			if(expr == nullptr)
				return nullptr;
			
			if(!this->check_token(token_t::RRB))
				return nullptr;
			
			return expr;
		}
		
		if(!this->check_token(token_t::ID, true, false))
			return nullptr;
		
		auto* node = factory->create_expr_symbol_ref(p);
		node->name = this->token().value;
		
		this->next_token();
		
		return node;
	}
	
	ast_expr_t* parser_impl_t::parse_literal_int(ast_node_t* p)
	{
		token_t& token = this->token();
		switch (token.type)
		{
			case token_t::BInt:
			{
				auto* node = factory->create_expr_int(p);
				node->value = String::binstrToValue<i32_t>(token.value);
				this->next_token();
				return node;
			}
			case token_t::XInt:
			{
				auto* node = factory->create_expr_int(p);
				node->value = String::hexstrToValue<i32_t>(token.value);
				this->next_token();
				return node;
			}
			case token_t::DInt:
			{
				auto* node = factory->create_expr_int(p);
				node->value = String::stringToValue<i32_t>(token.value);
				this->next_token();
				return node;
			}
			default:
				this->error_token_unexpected();
				return nullptr;
		}
	}
	
	ast_expr_t* parser_impl_t::parse_literal_float(ast_node_t* p)
	{
		token_t& token = this->token();
		switch (token.type)
		{
			case token_t::Float:
			{
				auto* node = factory->create_expr_float(p);
				node->value = String::stringToValue<f32_t>(token.value);
				this->next_token();
				return node;
			}
			default:
				this->error_token_unexpected();
				return nullptr;
		}
	}
	
	ast_expr_t* parser_impl_t::parse_literal_bool(ast_node_t* p)
	{
		token_t& token = this->token();
		switch (token.type)
		{
			case token_t::True:
			case token_t::False:
			{
				auto* node = factory->create_expr_bool(p);
				node->value = String::stringToValue<bool>(token.value);
				this->next_token();
				return node;
			}
			default:
				this->error_token_unexpected();
				return nullptr;
		}
	}
	
	ast_expr_t* parser_impl_t::parse_literal_string(ast_node_t* p)
	{
		token_t& token = this->token();
		switch (token.type)
		{
			case token_t::Str:
			{
				auto* node = factory->create_expr_string(p);
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
	ast_expr_t* parser_impl_t::parse_func_def(ast_node_t* p)
	{
		if(!this->check_token(token_t::Func))
			return nullptr;
		
		auto* node = factory->create_expr_func_def(p);
		if(!this->parse_func_params(node))
			return nullptr;
		
		// : ret-type
		if(!this->check_token(token_t::Colon))
			return nullptr;
		
		node->type = this->parse_type(node);
		if(node->type == nullptr)
			return nullptr;
		
		if(!this->parse_func_body(node))
			return nullptr;
		
		return node;
	}
	
	/*
	func_params => '(' [ID] ')'
	*/
	bool parser_impl_t::parse_func_params(ast_expr_func_def_t* node)
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
			
			if(!this->check_token(token_t::Colon))
				return false;
			
			ast_type_t* type = this->parse_type(node);
			if(type == nullptr)
				return false;
			
			node->addArg(name, type);
		} while (this->check_token(token_t::Comma, false));
		
		if(!this->check_token(token_t::RRB))
			return false;
		
		return true;
	}
	
	/*
	func_body => '{' [stat] '}'
	*/
	bool parser_impl_t::parse_func_body(ast_expr_func_def_t* node)
	{
		if(!this->check_token(token_t::LCB))
			return false;
		
		while (!this->check_token(token_t::RCB, false))
		{
			ast_stmt_t* stmt = this->parse_stmt(node);
			if(stmt == nullptr)
				return false;
			node->body.push_back(stmt);
		}
		
		return true;
	}
	
	/*
	func_call => '(' expr, expr, ..., expr ')'
	*/
	ast_expr_t* parser_impl_t::parse_func_call(ast_node_t* p, ast_expr_t* primary)
	{
		auto* node = factory->create_expr_func_ref(p);
		
		if(!this->check_token(token_t::LRB))
			return nullptr;
		
		while (!this->check_token(token_t::RRB, false))
		{
			if(!node->args.empty() && !this->check_token(token_t::Comma))
				return nullptr;
			
			ast_expr_t* arg = this->parse_expr(node);
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
	ast_expr_t* parser_impl_t::parse_object_def(ast_node_t* p)
	{
		if(!this->check_token(token_t::Make))
			return nullptr;
		
		auto* node = factory->create_expr_object_def(p);
		node->type = this->parse_type_ref(node);
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
		} while (this->check_token(token_t::Comma, false));
		
		if(!this->check_token(token_t::RCB))
			return nullptr;
		
		return node;
	}
	
	/*
	object_field => ID '=' expr
	*/
	bool parser_impl_t::parse_object_field(ast_expr_object_def_t* node)
	{
		if(!this->check_token(token_t::ID, true, false))
			return false;
		
		const String key = this->token().value;
		this->next_token();
		
		if(!this->check_token(token_t::Assign, false) && !this->check_token(token_t::Colon, false))
		{
			return false;
		}
		
		ast_expr_t* expr = this->parse_expr(node);
		if(expr == nullptr)
			return false;
		
		node->members[key] = expr;
		
		return true;
	}
	
	/*
	array_def => '[' [array_field {sep array_field} [sep]] ']'
	sep => ',' | ';'
	*/
	ast_expr_t* parser_impl_t::parse_array_def(ast_node_t* p)
	{
		if(!this->check_token(token_t::LSB))
			return nullptr;
		
		auto* node = factory->create_expr_array_def(p);
		
		do
		{
			if(this->token().type == token_t::RSB)
				break;
			
			ast_expr_t* expr = this->parse_expr(node);
			if(expr == nullptr)
				return nullptr;
			
			node->elements.push_back(expr);
		} while (this->check_token(token_t::Comma, false));
		
		if(!this->check_token(token_t::RSB))
			return nullptr;
		
		return node;
	}
	
	/*
	index_ref => '[' expr ']'
	*/
	ast_expr_t* parser_impl_t::parse_index_ref(ast_node_t* p, ast_expr_t* primary)
	{
		auto* node = factory->create_expr_index_ref(p);
		
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
	ast_expr_t* parser_impl_t::parse_object_ref(ast_node_t* p, ast_expr_t* primary)
	{
		auto* node = factory->create_expr_object_ref(p);
		
		if(!this->check_token(token_t::Dot))
			return nullptr;
		
		if(!this->check_token(token_t::ID, true, false))
			return nullptr;
		
		node->key = this->token().value;
		
		this->next_token();
		
		node->obj = primary;
		primary->parent = node;
		
		return node;
	}
	
	/*
	module_ref => 'using' '(' expr  ')'
	*/
	ast_expr_t* parser_impl_t::parse_module_ref(ast_node_t* p)
	{
		this->next_token(); // ignore 'using'
		
		if(!this->check_token(token_t::LRB))
			return nullptr;
		
		ast_expr_t* expr = this->parse_expr(p);
		if(expr == nullptr)
			return nullptr;
		
		if(!this->check_token(token_t::RRB))
			return nullptr;
		
		auto* node = factory->create_expr_module_ref(p);
		node->name = expr;
		
		return node;
	}
	
	
	ast_stmt_t* parser_impl_t::parse_stmt(ast_node_t* p)
	{
		ast_stmt_t* stmt = nullptr;
		bool semicolon = false;
		
		switch (this->token().type)
		{
			case token_t::Struct:
				stmt = this->parse_stmt_struct_def(p);
				break;
			case token_t::Enum:
				stmt = this->parse_stmt_enum_def(p);
				break;
			case token_t::Proc:
				stmt = this->parse_stmt_proc_def(p);
				semicolon = true;
				break;
			case token_t::Var:
			case token_t::Val:
				stmt = this->parse_stmt_symbol_def(p);
				semicolon = true;
				break;
			case token_t::Continue:
				stmt = this->parse_stmt_continue(p);
				semicolon = true;
				break;
			case token_t::Break:
				stmt = this->parse_stmt_break(p);
				semicolon = true;
				break;
			case token_t::Return:
				stmt = this->parse_stmt_return(p);
				semicolon = true;
				break;
			case token_t::If:
				stmt = this->parse_stmt_if(p);
				break;
			case token_t::Loop:
				stmt = this->parse_stmt_loop(p);
				break;
			case token_t::LCB:
				stmt = this->parse_stmt_block(p);
				break;
			default:
				stmt = this->parse_stmt_assign_or_call(p);
				semicolon = true;
		}
		
		if(semicolon && !this->check_token(token_t::Semicolon))
			return nullptr;
		
		return stmt;
	}
	
	/**
	 * struct_def := 'struct' ID [':' type_ref] '{' struct_member '};';
	*/
	ast_stmt_struct_def_t* parser_impl_t::parse_stmt_struct_def(ast_node_t* p)
	{
		if(!this->check_token(token_t::Struct))
			return nullptr;
		
		auto* node = factory->create_stmt_struct_def(p);
		
		// ID
		if(!this->check_token(token_t::ID, true, false))
			return nullptr;
		
		node->name = this->token().value;
		this->next_token();
		
		// : base
		if(this->check_token(token_t::Colon, false))
		{
			node->base = this->parse_type_ref(node);
			if(node->base == nullptr)
				return nullptr;
		}
		
		// {
		if(!this->check_token(token_t::LCB))
			return nullptr;
		
		do
		{
			if(this->token().type == token_t::RCB)
				break;
			
			ast_stmt_struct_member_t* member = this->parse_stmt_struct_member(node);
			if(member == nullptr)
				return nullptr;
			
			const String& name = member->name;
			if(node->members.find(name) != node->members.end())
			{
				this->error_token_unexpected();
				return nullptr;
			}
			
			node->members[name] = member;
		} while (this->check_token(token_t::Semicolon, false));
		
		// }
		if(!this->check_token(token_t::RCB))
			return nullptr;
		
		return node;
	}
	
	/**
	 * struct_member := ['static'] ('var' | 'val') ID : type [ '=' expr ] ;
	*/
	ast_stmt_struct_member_t* parser_impl_t::parse_stmt_struct_member(ast_node_t* p)
	{
		auto* node = factory->create_stmt_struct_member(p);
		
		// [static]
		node->isStatic = this->check_token(token_t::Static, false);
		
		// (val | var)
		switch(this->token().type)
		{
			case token_t::Var:
				node->isConst = false; break;
			case token_t::Val:
				node->isConst = true; break;
			default:
				this->error_token_unexpected();
				return nullptr;
		}
		this->next_token();
		
		// ID
		if(!this->check_token(token_t::ID, true, false))
			return nullptr;
		
		node->name = this->token().value;
		
		this->next_token();
		
		// : type
		if(!this->check_token(token_t::Colon))
			return nullptr;
		node->type = this->parse_type(node);
		if(node->type == nullptr)
			return nullptr;
		
		// [= expr]
		if(this->check_token(token_t::Assign, false))
		{
			node->value = this->parse_expr(node);
			if(node->value == nullptr)
				return nullptr;
		}
		
		return node;
	}
	
	/**
	 * enum_def := 'enum' ID '{' [enum_member] '}' ';';
	 * enum_member := ID ['=' expr_int] ',';
	 * */
	ast_stmt_enum_def_t* parser_impl_t::parse_stmt_enum_def(ast_node_t* p)
	{
		if(!this->check_token(token_t::Enum))
			return nullptr;
		
		auto* node = factory->create_stmt_enum_def(p);
		
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
			if(this->check_token(token_t::Assign, false))
			{
				auto memExpr = this->parse_literal_int(node);
				if(memExpr == nullptr)
					return nullptr;
				auto memIntExpr = dynamic_cast<ast_expr_int_t*>(memExpr);
				memValue = static_cast<i32_t>(memIntExpr->value);
				index = memValue;
			}
			
			node->members[memName] = memValue;
			index += 1;
			
		} while (this->check_token(token_t::Comma, false));
		
		// }
		if(!this->check_token(token_t::RCB))
			return nullptr;
		
		return node;
	}
	
	/**
	 * proc_def := 'proc' ID '(' [func_params]* ')' ';';
	 * func_params := ID ':' type_ref ','
	*/
	ast_stmt_proc_def_t* parser_impl_t::parse_stmt_proc_def(ast_node_t* p)
	{
		if(!this->check_token(token_t::Proc))
			return nullptr;
		
		auto* node = factory->create_stmt_proc_def(p);
		
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
			if(!this->check_token(token_t::Colon))
				return nullptr;
			ast_type_t* argType = this->parse_type(node);
			if(argType == nullptr)
				return nullptr;
			
			node->args[argName] = argType;
		} while (this->check_token(token_t::Comma, false));
		
		// )
		if(!this->check_token(token_t::RRB))
			return nullptr;
		
		// : ret-type
		if(!this->check_token(token_t::Colon))
			return nullptr;
		node->type = this->parse_type(node);
		if(node->type == nullptr)
			return nullptr;
		
		return node;
	}
	
	/**
	 * symbol_def := 'var' | 'val' ID [: type] = expr;
	*/
	ast_stmt_symbol_def_t* parser_impl_t::parse_stmt_symbol_def(ast_node_t* p)
	{
		if(!this->check_token(token_t::Var, false, false) && !this->check_token(token_t::Val, false, false))
		{
			return nullptr;
		}
		
		auto* node = factory->create_stmt_symbol_def(p);
		node->variable = this->token().type == token_t::Var;
		
		this->next_token(); // ignore 'var' | 'val'
		
		if(!this->check_token(token_t::ID, true, false))
			return nullptr;
		
		node->name = this->token().value;
		
		this->next_token();
		
		// : type
		if(this->check_token(token_t::Colon, false))
		{
			node->type = this->parse_type(p);
			if(node->type == nullptr)
				return nullptr;
		}
		
		// = expr
		if(!this->check_token(token_t::Assign, true))
			return nullptr;
		
		node->value = this->parse_expr(node);
		if(node->value == nullptr)
			return nullptr;
		
		return node;
	}
	
	ast_stmt_continue_t* parser_impl_t::parse_stmt_continue(ast_node_t* p)
	{
		if(!this->check_token(token_t::Continue))
			return nullptr;
		
		auto* node = factory->create_stmt_continue(p);
		
		return node;
	}
	
	ast_stmt_break_t* parser_impl_t::parse_stmt_break(ast_node_t* p)
	{
		if(!this->check_token(token_t::Break))
			return nullptr;
		
		auto* node = factory->create_stmt_break(p);
		
		return node;
	}
	
	ast_stmt_return_t* parser_impl_t::parse_stmt_return(ast_node_t* p)
	{
		if(!this->check_token(token_t::Return))
			return nullptr;
		
		auto* node = factory->create_stmt_return(p);
		
		if(this->check_token(token_t::Semicolon, false, false))
			return node;
		
		node->value = this->parse_expr(node);
		if(node->value == nullptr)
			return nullptr;
		
		return node;
	}
	
	ast_stmt_if_t* parser_impl_t::parse_stmt_if(ast_node_t* p)
	{
		if(!this->check_token(token_t::If))
			return nullptr;
		
		auto* node = factory->create_stmt_if(p);
		
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
		
		if(this->check_token(token_t::Else, false))
		{
			node->branch_false = this->parse_stmt(node);
			if(node->branch_false == nullptr)
				return nullptr;
		}
		
		return node;
	}
	
	ast_stmt_loop_t* parser_impl_t::parse_stmt_loop(ast_node_t* p)
	{
		if(!this->check_token(token_t::Loop))
			return nullptr;
		
		auto* node = factory->create_stmt_loop(p);
		
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
	
	ast_stmt_t* parser_impl_t::parse_stmt_loop_init(ast_node_t* p)
	{
		ast_stmt_t* stmt = this->parse_stmt_symbol_def(p);
		if(stmt == nullptr)
			return nullptr;
		
		if(!this->check_token(token_t::Semicolon))
			return nullptr;
		
		return stmt;
	}
	
	ast_expr_t* parser_impl_t::parse_stmt_loop_cond(ast_node_t* p)
	{
		ast_expr_t* cond = this->parse_expr(p);
		if(cond == nullptr)
			return nullptr;
		
		if(!this->check_token(token_t::Semicolon))
			return nullptr;
		
		return cond;
	}
	
	ast_stmt_t* parser_impl_t::parse_stmt_loop_step(ast_node_t* p)
	{
		ast_stmt_t* stmt = nullptr;
		switch (this->token().type)
		{
			case token_t::Var:
			case token_t::Val:
				stmt = this->parse_stmt_symbol_def(p);
				break;
			default:
				stmt = this->parse_stmt_assign_or_call(p);
				break;
		}
		
		return stmt;
	}
	
	ast_stmt_block_t* parser_impl_t::parse_stmt_block(ast_node_t* p)
	{
		if(!this->check_token(token_t::LCB))
			return nullptr;
		
		auto* node = factory->create_stmt_block(p);
		
		while (!this->check_token(token_t::RCB, false))
		{
			ast_stmt_t* stmt = this->parse_stmt(node);
			if(stmt == nullptr)
				return nullptr;
			
			node->stmts.push_back(stmt);
			
			this->check_token(token_t::Semicolon, false);
		}
		
		return node;
	}
	
	ast_stmt_t* parser_impl_t::parse_stmt_assign_or_call(ast_node_t* p)
	{
		ast_expr_t* left = this->parse_expr_suffixed(p);
		if(left == nullptr)
			return nullptr;
		
		if(this->check_token(token_t::Assign, false))
		{
			auto* node = factory->create_stmt_assign(p);
			node->left = left;
			left->parent = node;
			
			node->right = this->parse_expr(node);
			if(node->right == nullptr)
				return nullptr;
			
			return node;
		}
		else if(left->category == ast_node_category_t::expr_func_ref)
		{
			auto* node = factory->create_stmt_call(p);
			node->expr = dynamic_cast<ast_expr_func_ref_t*>(left);
			left->parent = node;
			
			return node;
		}
		else
		{
			this->error_token_unexpected();
			return nullptr;
		}
	}
	
	void parser_impl_t::next_token()
	{
		return scanner->next_token();
	}
	
	token_t& parser_impl_t::token()
	{
		return scanner->token();
	}
	
	token_t& parser_impl_t::look_ahead_token()
	{
		return scanner->look_ahead_token();
	}
	
	bool parser_impl_t::check_token(const token_t::token_type& tokenType, bool required, bool movenext)
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
	
	ast_unary_oper_t parser_impl_t::check_unary_oper(bool required, bool movenext)
	{
		ast_unary_oper_t oper = ast_unary_oper_t::Unknown;
		switch (scanner->token().type)
		{
			case token_t::Add:
				oper = ast_unary_oper_t::Pos;
				break;
			case token_t::Sub:
				oper = ast_unary_oper_t::Neg;
				break;
			case token_t::Not:
				oper = ast_unary_oper_t::Not;
				break;
			case token_t::Flip:
				oper = ast_unary_oper_t::Flip;
				break;
			case token_t::At:
				oper = ast_unary_oper_t::TypeOf;
				break;
			case token_t::Pound:
				oper = ast_unary_oper_t::SizeOf;
				break;
			default:
				break;
		}
		if(oper == ast_unary_oper_t::Unknown)
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
	
	ast_binary_oper_t parser_impl_t::check_binary_oper(int priority, bool required, bool movenext)
	{
		ast_binary_oper_t oper = ast_binary_oper_t::Unknown;
		switch (scanner->token().type)
		{
			case token_t::Or2:
				oper = ast_binary_oper_t::Or;
				break;
			case token_t::And2:
				oper = ast_binary_oper_t::And;
				break;
			case token_t::Is:
				oper = ast_binary_oper_t::Is;
				break;
			case token_t::Equal:
				oper = ast_binary_oper_t::Equal;
				break;
			case token_t::Greater:
				oper = ast_binary_oper_t::Greater;
				break;
			case token_t::Less:
				oper = ast_binary_oper_t::Less;
				break;
			case token_t::GEqual:
				oper = ast_binary_oper_t::GEqual;
				break;
			case token_t::LEqual:
				oper = ast_binary_oper_t::LEqual;
				break;
			case token_t::NEqual:
				oper = ast_binary_oper_t::NEqual;
				break;
			case token_t::Add:
				oper = ast_binary_oper_t::Add;
				break;
			case token_t::Sub:
				oper = ast_binary_oper_t::Sub;
				break;
			case token_t::Mul:
				oper = ast_binary_oper_t::Mul;
				break;
			case token_t::Div:
				oper = ast_binary_oper_t::Div;
				break;
			case token_t::Mod:
				oper = ast_binary_oper_t::Mod;
				break;
			case token_t::And:
				oper = ast_binary_oper_t::BitAnd;
				break;
			case token_t::Or:
				oper = ast_binary_oper_t::BitOr;
				break;
			case token_t::Xor:
				oper = ast_binary_oper_t::BitXor;
				break;
			case token_t::ShiftL:
				oper = ast_binary_oper_t::ShiftL;
				break;
			case token_t::ShiftR:
				oper = ast_binary_oper_t::ShiftR;
				break;
			case token_t::As:
				oper = ast_binary_oper_t::As;
				break;
			default:
				break;
		}
		if((int) oper / 100 != priority)
		{
			oper = ast_binary_oper_t::Unknown;
		}
		if(oper == ast_binary_oper_t::Unknown)
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
	
	const String& parser_impl_t::error()
	{
		return errormsg;
	}
	
	void parser_impl_t::error(const char* fmt, ...)
	{
		String message;
		_FormatVA(message, fmt);
		errormsg = String::format("%s at %d, %d.\n", message.cstr(), scanner->line(), scanner->column());
	}
	
	void parser_impl_t::error_token_unexpected()
	{
		token_t& token = scanner->token();
		const char* value = token.value.cstr();
		if(token.type == token_t::Eos)
		{
			this->error("unexpected eos");
		}
		else
		{
			this->error("unexpected token '%s'", value);
		}
	}
	
	parser_t::parser_t() : impl(new parser_impl_t())
	{
	}
	
	parser_t::~parser_t()
	{
		_DeletePointer(this->impl);
	}
	
	ast_module_t* parser_t::parse(const char* source)
	{
		return this->impl->parse_module(source);
	}
	
	const String& parser_t::error() const
	{
		return this->impl->error();
	}
_EndNamespace(eokas)
