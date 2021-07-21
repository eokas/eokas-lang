
#include "parser.h"
#include "scanner.h"
#include "ast.h"

_BeginNamespace(eokas)

class parser_impl_t
{
public:
	parser_impl_t();
	virtual ~parser_impl_t();

public:
	void clear();

	ast_module_t* parse_module(const char* source);

	ast_type_ref_t* parse_type_ref(ast_node_t* p);

	ast_expr_t* parse_expr(ast_node_t* p);
	ast_expr_t* parse_expr_trinary(ast_node_t* p);
	ast_expr_t* parse_expr_binary(ast_node_t* p, int priority = 1);
	ast_expr_t* parse_expr_unary(ast_node_t* p);
	ast_expr_t* parse_expr_suffixed(ast_node_t* p);
	ast_expr_t* parse_expr_primary(ast_node_t* p);
	ast_expr_t* parse_func_def(ast_node_t* p);
	bool parse_func_params(ast_expr_func_def_t* node);
	bool parse_func_body(ast_expr_func_def_t* node);
	ast_expr_t* parse_object_def(ast_node_t* p);
	ast_expr_t* parse_func_call(ast_node_t* p, ast_expr_t* primary);
	ast_expr_t* parse_array_def(ast_node_t* p);
	ast_expr_t* parse_index_ref(ast_node_t* p, ast_expr_t* primary);
	bool parse_object_field(ast_expr_obj_def_t* node);
	ast_expr_t* parse_object_ref(ast_node_t* p, ast_expr_t* primary);
	ast_expr_t* parse_module_ref(ast_node_t* p);

	ast_stmt_t* parse_stmt(ast_node_t* p);
	ast_stmt_schema_def_t* parse_stmt_schema_def(ast_node_t* p);
	ast_stmt_schema_member_t* parse_stmt_schema_member(ast_node_t* p);
	ast_stmt_struct_def_t* parse_stmt_struct_def(ast_node_t* p);
	ast_stmt_struct_member_t* parse_stmt_struct_member(ast_node_t* p);
	ast_stmt_proc_def_t* parse_stmt_proc_def(ast_node_t* p);
	ast_stmt_symbol_def_t* parse_stmt_symbol_def(ast_node_t* p);
	ast_stmt_continue_t* parse_stmt_continue(ast_node_t* p);
	ast_stmt_break_t* parse_stmt_break(ast_node_t* p);
	ast_stmt_return_t* parse_stmt_return(ast_node_t* p);
	ast_stmt_if_t* parse_stmt_if(ast_node_t* p);
	ast_stmt_while_t* parse_stmt_while(ast_node_t* p);
	ast_stmt_for_t* parse_stmt_for(ast_node_t* p);
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
	class scanner_t* m_scanner;
	struct ast_node_t* m_ast;
	String m_error;
};


parser_impl_t::parser_impl_t()
	: m_scanner(new scanner_t())
	, m_ast(nullptr)
	, m_error()
{ }

parser_impl_t::~parser_impl_t()
{
	this->clear();
	_DeletePointer(this->m_scanner);
}

void parser_impl_t::clear()
{
	_DeletePointer(this->m_ast);
	this->m_error.clear();
	this->m_scanner->clear();
}

ast_module_t* parser_impl_t::parse_module(const char* source)
{
	this->clear();
	this->m_scanner->ready(source);

	ast_module_t* node = new ast_module_t();

	this->next_token();
	while (this->token().type != token_t::Eos)
	{
		ast_stmt_t* stmt = this->parse_stmt(node);
		if (stmt == nullptr)
			return nullptr;
		node->stmts.push_back(stmt);
	}

	return node;
}

ast_type_ref_t* parser_impl_t::parse_type_ref(ast_node_t* p)
{
	if (!this->check_token(token_t::ID, true, false))
		return nullptr;

	ast_type_ref_t* node = new ast_type_ref_t(p);
	node->name = this->token().value;
	this->next_token(); // ignore ID

	return node;
}

ast_expr_t* parser_impl_t::parse_expr(ast_node_t* p)
{
	return this->parse_expr_trinary(p);
}

ast_expr_t* parser_impl_t::parse_expr_trinary(ast_node_t* p)
{
	ast_expr_t* binary = this->parse_expr_binary(p, 1);
	if (binary == nullptr)
	{
		_DeletePointer(binary);
		return nullptr;
	}

	if (this->check_token(token_t::Question, false))
	{
		ast_expr_trinary_t* trinary = new ast_expr_trinary_t(p);
		trinary->cond = binary;
		binary->parent = trinary;

		ast_expr_t* expr_true = this->parse_expr(trinary);
		if (expr_true == nullptr)
		{
			_DeletePointer(trinary);
			return nullptr;
		}

		if (!this->check_token(token_t::Colon))
		{
			_DeletePointer(trinary);
			return nullptr;
		}

		ast_expr_t* expr_false = this->parse_expr(trinary);
		if (expr_false == nullptr)
		{
			_DeletePointer(trinary);
			return nullptr;
		}

		return trinary;
	}

	return binary;
}

ast_expr_t* parser_impl_t::parse_expr_binary(ast_node_t* p, int priority)
{
	ast_expr_t* left = nullptr;

	if (priority < (int)ast_binary_oper_t::MaxPriority / 100)
	{
		left = this->parse_expr_binary(p, priority + 1);
	}
	else
	{
		left = this->parse_expr_unary(p);
	}
	if (left == nullptr)
	{
		return nullptr;
	}

	for (;;)
	{
		ast_binary_oper_t oper = this->check_binary_oper(priority, false);
		if (oper == ast_binary_oper_t::Unknown)
			break;

		ast_expr_t* right = nullptr;
		if (priority < (int)ast_binary_oper_t::MaxPriority / 100)
		{
			right = this->parse_expr_binary(p, priority + 1);
		}
		else
		{
			right = this->parse_expr_unary(p);
		}
		if (right == nullptr)
		{
			_DeletePointer(left);
			return nullptr;
		}

		ast_expr_binary_t* binary = new ast_expr_binary_t(p);
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
ast_expr_t* parser_impl_t::parse_expr_unary(ast_node_t* p)
{
	ast_unary_oper_t oper = this->check_unary_oper(false, false);

	ast_expr_t* right = nullptr;

	token_t& token = this->token();
	switch (token.type)
	{
	case token_t::BInt:
	{
		ast_expr_int_t* node = new ast_expr_int_t(p);
		node->value = String::binstrToValue<inum_t>(token.value);
		right = node;
		this->next_token();
	}
	break;
	case token_t::XInt:
	{
		ast_expr_int_t* node = new ast_expr_int_t(p);
		node->value = String::hexstrToValue<inum_t>(token.value);
		right = node;
		this->next_token();
	}
	break;
	case token_t::DInt:
	{
		ast_expr_int_t* node = new ast_expr_int_t(p);
		node->value = String::stringToValue<inum_t>(token.value);
		right = node;
		this->next_token();
	}
	break;
	case token_t::Float:
	{
		ast_expr_float_t* node = new ast_expr_float_t(p);
		node->value = String::stringToValue<fnum_t>(token.value);
		right = node;
		this->next_token();
	}
	break;
	case token_t::Str:
	{
		ast_expr_string_t* node = new ast_expr_string_t(p);
		node->value = token.value;
		right = node;
		this->next_token();
	}
	break;
	case token_t::True:
	case token_t::False:
	{
		ast_expr_bool_t* node = new ast_expr_bool_t(p);
		node->value = String::stringToValue<bool>(token.value);
		right = node;
		this->next_token();
	}
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
	case token_t::ID: case token_t::LRB:
		right = this->parse_expr_suffixed(p);
		break;
	default:
		this->error_token_unexpected();
		return nullptr;
	}

	if (oper == ast_unary_oper_t::Unknown)
		return right;

	ast_expr_unary_t* unary = new ast_expr_unary_t(p);
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
	if (primary == nullptr)
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

		if (suffixed == nullptr)
		{
			_DeletePointer(primary);
			return nullptr;
		}

		primary = suffixed;
	}

	return primary;
}

/*
expr_primary := ID | '(' expr ')'
*/
ast_expr_t* parser_impl_t::parse_expr_primary(ast_node_t* p)
{
	if (this->check_token(token_t::LRB, false))
	{
		ast_expr_t* expr = this->parse_expr(p);
		if (expr == nullptr)
			return nullptr;

		if (!this->check_token(token_t::RRB))
		{
			_DeletePointer(expr);
			return nullptr;
		}

		return expr;
	}

	if (!this->check_token(token_t::ID, true, false))
		return nullptr;

	ast_expr_symbol_ref_t* node = new ast_expr_symbol_ref_t(p);
	node->name = this->token().value;

	this->next_token();

	return node;
}

/*
func_def => 'func' func_params func_body
*/
ast_expr_t* parser_impl_t::parse_func_def(ast_node_t* p)
{
	if (!this->check_token(token_t::Func))
		return nullptr;

	ast_expr_func_def_t* node = new ast_expr_func_def_t(p);
	if (!this->parse_func_params(node))
	{
		_DeletePointer(node);
		return nullptr;
	}

	// : ret-type
	if (!this->check_token(token_t::Colon))
	{
		_DeletePointer(node);
		return nullptr;
	}
	node->type = this->parse_type_ref(node);
	if (node->type == nullptr)
	{
		_DeletePointer(node);
		return nullptr;
	}

	if (!this->parse_func_body(node))
	{
		_DeletePointer(node);
		return nullptr;
	}

	return node;
}

/*
func_params => '(' [ID] ')'
*/
bool parser_impl_t::parse_func_params(ast_expr_func_def_t* node)
{
	if (!this->check_token(token_t::LRB)) // (
		return false;
	do
	{
		if (this->token().type == token_t::RRB)
			break;

		if (!this->check_token(token_t::ID, true, false))
			return false;
		const String name = this->token().value;
		if(node->getArg(name) != nullptr)
		{
			this->error_token_unexpected();
			return false;
		}
		this->next_token();

		if (!this->check_token(token_t::Colon))
			return false;

		ast_type_t* type = this->parse_type_ref(node);
		if (type == nullptr)
			return false;

		ast_expr_func_def_t::arg_t arg;
		arg.name = name;
		arg.type = type;
		node->args.push_back(arg);
	}
	while (this->check_token(token_t::Comma, false));

	if (!this->check_token(token_t::RRB))
		return false;

	return true;
}

/*
func_body => '{' [stat] '}'
*/
bool parser_impl_t::parse_func_body(ast_expr_func_def_t* node)
{
	if (!this->check_token(token_t::LCB))
		return false;

	while (!this->check_token(token_t::RCB, false))
	{
		ast_stmt_t* stmt = this->parse_stmt(node);
		if (stmt == nullptr)
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
	ast_expr_func_ref_t* node = new ast_expr_func_ref_t(p);
	node->func = primary;
	primary->parent = node;

	if (!this->check_token(token_t::LRB))
	{
		_DeletePointer(node);
		return nullptr;
	}

	while (!this->check_token(token_t::RRB, false))
	{
		if (!node->args.empty() && !this->check_token(token_t::Comma))
		{
			_DeletePointer(node);
			return nullptr;
		}

		ast_expr_t* arg = this->parse_expr(node);
		if (arg == nullptr)
		{
			_DeletePointer(node);
			return nullptr;
		}

		node->args.push_back(arg);
	}

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
	
	ast_expr_obj_def_t* node = new ast_expr_obj_def_t(p);
	node->type = this->parse_type_ref(node);
	if (node->type == nullptr)
	{
		_DeletePointer(node);
		return nullptr;
	}

	if (!this->check_token(token_t::LCB))
	{
		_DeletePointer(node);
		return nullptr;
	}

	do
	{
		if (this->token().type == token_t::RCB)
			break;
		if (!this->parse_object_field(node))
		{
			_DeletePointer(node);
			return nullptr;
		}
	}
	while (this->check_token(token_t::Comma, false));

	if (!this->check_token(token_t::RCB))
	{
		_DeletePointer(node);
		return nullptr;
	}

	return node;
}


/*
array_def => '[' [array_field {sep array_field} [sep]] ']'
sep => ',' | ';'
*/
ast_expr_t* parser_impl_t::parse_array_def(ast_node_t* p)
{
	if (!this->check_token(token_t::LSB))
		return nullptr;

	ast_expr_array_def_t* node = new ast_expr_array_def_t(p);

	do
	{
		if (this->token().type == token_t::RSB)
			break;
		ast_expr_t* expr = this->parse_expr(node);
		if (expr == nullptr)
		{
			_DeletePointer(node);
			return nullptr;
		}
		node->items.push_back(expr);
	}
	while (this->check_token(token_t::Comma, false));

	if (!this->check_token(token_t::RSB))
	{
		_DeletePointer(node);
		return nullptr;
	}

	return node;
}

/*
index_ref => '[' expr ']'
*/
ast_expr_t* parser_impl_t::parse_index_ref(ast_node_t* p, ast_expr_t* primary)
{
	ast_expr_index_ref_t* node = new ast_expr_index_ref_t(p);
	node->obj = primary;
	primary->parent = node;

	if (!this->check_token(token_t::LSB))
	{
		_DeletePointer(node);
		return nullptr;
	}

	node->key = this->parse_expr(node);
	if (node->key == nullptr)
	{
		_DeletePointer(node);
		return nullptr;
	}

	if (!this->check_token(token_t::RSB))
	{
		_DeletePointer(node);
		return nullptr;
	}

	return node;
}

/*
object_field => ID '=' expr
*/
bool parser_impl_t::parse_object_field(ast_expr_obj_def_t* node)
{
	if (!this->check_token(token_t::ID, true, false))
		return false;

	const String key = this->token().value;
	this->next_token();

	if (!this->check_token(token_t::Assign, false) &&
		!this->check_token(token_t::Colon, false))
	{
		return false;
	}

	ast_expr_t* expr = this->parse_expr(node);
	if (expr == nullptr)
		return false;

	node->members[key] = expr;

	return true;
}

/*
object_ref => '.' ID
*/
ast_expr_t* parser_impl_t::parse_object_ref(ast_node_t* p, ast_expr_t* primary)
{
	ast_expr_obj_ref_t* node = new ast_expr_obj_ref_t(p);
	node->obj = primary;
	primary->parent = node;

	if (!this->check_token(token_t::Dot))
	{
		_DeletePointer(node);
		return nullptr;
	}

	if (!this->check_token(token_t::ID, true, false))
	{
		_DeletePointer(node);
		return nullptr;
	}

	node->key = this->token().value;

	this->next_token();

	return node;
}

/*
module_ref => 'using' '(' expr  ')'
*/
ast_expr_t* parser_impl_t::parse_module_ref(ast_node_t* p)
{
	this->next_token(); // ignore 'using'

	if (!this->check_token(token_t::LRB))
		return nullptr;

	ast_expr_t* expr = this->parse_expr(p);
	if (expr == nullptr)
		return nullptr;

	if (!this->check_token(token_t::RRB))
		return nullptr;

	ast_expr_module_ref_t* node = new ast_expr_module_ref_t(p);
	node->name = expr;

	return node;
}


ast_stmt_t* parser_impl_t::parse_stmt(ast_node_t* p)
{
	switch (this->token().type)
	{
	case token_t::Schema:
		return this->parse_stmt_schema_def(p);
	case token_t::Struct:
		return this->parse_stmt_struct_def(p);
	case token_t::Var:
	case token_t::Val:
		return this->parse_stmt_symbol_def(p);
	case token_t::Continue:
		return this->parse_stmt_continue(p);
	case token_t::Break:
		return this->parse_stmt_break(p);
	case token_t::Return:
		return this->parse_stmt_return(p);
	case token_t::If:
		return this->parse_stmt_if(p);
	case token_t::While:
		return this->parse_stmt_while(p);
	case token_t::For:
		return this->parse_stmt_for(p);
	case token_t::LCB:
		return this->parse_stmt_block(p);
	default:
		if (this->token().type == token_t::ID &&
			this->look_ahead_token().type == token_t::Colon)
		{
			return this->parse_stmt_symbol_def(p);
		}
		else
		{
			return this->parse_stmt_assign_or_call(p);
		}
	}
}

/**
 * schema_def := 'schema' ID [':' type_ref] '{' schema_member '};';
*/
ast_stmt_schema_def_t* parser_impl_t::parse_stmt_schema_def(ast_node_t* p)
{
	if (this->check_token(token_t::Schema))
		return nullptr;

	ast_stmt_schema_def_t* node = new ast_stmt_schema_def_t(p);

	// ID
	if (!this->check_token(token_t::ID, true, false))
	{
		_DeletePointer(node);
		return nullptr;
	}
	node->name = this->token().value;
	this->next_token();

	// : schema_ref
	if (this->check_token(token_t::Colon, false))
	{
		node->schema = this->parse_type_ref(node);
		if (node->schema == nullptr)
		{
			_DeletePointer(node);
			return nullptr;
		}
	}

	// {
	if (!this->check_token(token_t::LCB))
	{
		_DeletePointer(node);
		return nullptr;
	}

	do
	{
		if (this->token().type == token_t::RCB)
			break;

		ast_stmt_schema_member_t* member = this->parse_stmt_schema_member(node);
		if (member == nullptr)
		{
			_DeletePointer(node);
			return nullptr;
		}

		const String& name = member->name;
		if (node->members.find(name) != node->members.end())
		{
			_DeletePointer(member);
			_DeletePointer(node);
			this->error_token_unexpected();
			return nullptr;
		}

		node->members[name] = member;
	}
	while (this->check_token(token_t::Comma, false));

	// }
	if (!this->check_token(token_t::RCB))
	{
		_DeletePointer(node);
		return nullptr;
	}

	// ;
	if (!this->check_token(token_t::Semicolon, true))
	{
		_DeletePointer(node);
		return nullptr;
	}

	return node;
}

/**
 * schema_member := ID : type;
*/
ast_stmt_schema_member_t* parser_impl_t::parse_stmt_schema_member(ast_node_t* p)
{
	ast_stmt_schema_member_t* node = new ast_stmt_schema_member_t(p);

	if (this->check_token(token_t::ID, true, false))
	{
		_DeletePointer(node);
		return nullptr;
	}

	node->name = this->token().value;

	this->next_token();

	// : type
	if (!this->check_token(token_t::Colon))
	{
		_DeletePointer(node);
		return nullptr;
	}
	node->type = this->parse_type_ref(p);
	if (node->type == nullptr)
	{
		_DeletePointer(node);
		return nullptr;
	}

	// ;
	if (!this->check_token(token_t::Semicolon, true))
	{
		_DeletePointer(node);
		return nullptr;
	}

	return node;
}

/**
 * struct_def := 'struct' ID [':' type_ref] '{' struct_member '};';
*/
ast_stmt_struct_def_t* parser_impl_t::parse_stmt_struct_def(ast_node_t* p)
{
	if (this->check_token(token_t::Struct))
		return nullptr;

	ast_stmt_struct_def_t* node = new ast_stmt_struct_def_t(p);

	// ID
	if (!this->check_token(token_t::ID, true, false))
	{
		_DeletePointer(node);
		return nullptr;
	}
	node->name = this->token().value;
	this->next_token();

	// : schema_ref
	if (this->check_token(token_t::Colon, false))
	{
		node->schema = this->parse_type_ref(node);
		if (node->schema == nullptr)
		{
			_DeletePointer(node);
			return nullptr;
		}
	}

	// {
	if (!this->check_token(token_t::LCB))
	{
		_DeletePointer(node);
		return nullptr;
	}

	do
	{
		if (this->token().type == token_t::RCB)
			break;

		ast_stmt_struct_member_t* member = this->parse_stmt_struct_member(node);
		if (member == nullptr)
		{
			_DeletePointer(node);
			return nullptr;
		}

		const String& name = member->name;
		if (node->getMember(name) != nullptr)
		{
			_DeletePointer(member);
			_DeletePointer(node);
			this->error_token_unexpected();
			return nullptr;
		}

		node->members.push_back(member);
	}
	while (this->check_token(token_t::Comma, false));

	// }
	if (!this->check_token(token_t::RCB))
	{
		_DeletePointer(node);
		return nullptr;
	}

	// ;
	if (!this->check_token(token_t::Semicolon, true))
	{
		_DeletePointer(node);
		return nullptr;
	}

	return node;
}

/**
 * struct_member := ID [: type] = expr;
*/
ast_stmt_struct_member_t* parser_impl_t::parse_stmt_struct_member(ast_node_t* p)
{
	ast_stmt_struct_member_t* node = new ast_stmt_struct_member_t(p);

	this->next_token(); // ignore 'var' | 'val'

	if (!this->check_token(token_t::ID, true, false))
	{
		_DeletePointer(node);
		return nullptr;
	}

	node->name = this->token().value;

	this->next_token();

	// : type
	if (this->check_token(token_t::Colon, false))
	{
		node->type = this->parse_type_ref(p);
		if (node->type == nullptr)
		{
			_DeletePointer(node);
			return nullptr;
		}
	}

	// = expr
	if (!this->check_token(token_t::Assign, true))
	{
		_DeletePointer(node);
		return nullptr;
	}
	node->value = this->parse_expr(node);
	if (node->value == nullptr)
	{
		_DeletePointer(node);
		return nullptr;
	}

	// ;
	if (!this->check_token(token_t::Semicolon, true))
	{
		_DeletePointer(node);
		return nullptr;
	}

	return node;
}

/**
 * proc_def := 'proc' ID '(' [func_params]* ')' ';';
 * func_params := ID ':' type_ref ','
*/
ast_stmt_proc_def_t* parser_impl_t::parse_stmt_proc_def(ast_node_t* p)
{
	if (!this->check_token(token_t::Proc))
		return nullptr;

	ast_stmt_proc_def_t* node = new ast_stmt_proc_def_t(p);

	// ID
	if (!this->check_token(token_t::ID, true, false))
	{
		_DeletePointer(node);
		return nullptr;
	}
	node->name = this->token().value;
	this->next_token();

	// (
	if (!this->check_token(token_t::LRB))
	{
		_DeletePointer(node);
		return nullptr;
	}

	do
	{
		if (this->token().type == token_t::RRB)
			break;

		// ID
		if (!this->check_token(token_t::ID, true, false))
		{
			_DeletePointer(node);
			return nullptr;
		}
		const String argName = this->token().value;
		if (node->args.find(argName) != node->args.end())
		{
			_DeletePointer(node);
			this->error_token_unexpected();
			return nullptr;
		}
		this->next_token();

		// :
		if(!this->check_token(token_t::Colon))
		{
			_DeletePointer(node);
			return nullptr;
		}

		// type
		ast_type_ref_t* argType = this->parse_type_ref(p);
		if(argType == nullptr)
		{
			_DeletePointer(node);
			return nullptr;
		}

		node->args[argName] = argType;
	}
	while (this->check_token(token_t::Comma, false));

	// )
	if (!this->check_token(token_t::RRB))
	{
		_DeletePointer(node);
		return nullptr;
	}

	// ;
	if (!this->check_token(token_t::Semicolon, true))
	{
		_DeletePointer(node);
		return nullptr;
	}

	return node;
}

/**
 * symbol_def := 'var' | 'val' ID [: type] = expr;
*/
ast_stmt_symbol_def_t* parser_impl_t::parse_stmt_symbol_def(ast_node_t* p)
{
	if (!this->check_token(token_t::Var, false, false) &&
		!this->check_token(token_t::Val, false, false))
	{
		return nullptr;
	}

	ast_stmt_symbol_def_t* node = new ast_stmt_symbol_def_t(p);
	node->variable = this->token().type == token_t::Var;

	this->next_token(); // ignore 'var' | 'val'

	if (!this->check_token(token_t::ID, true, false))
	{
		_DeletePointer(node);
		return nullptr;
	}

	node->name = this->token().value;

	this->next_token();

	// : type
	if (this->check_token(token_t::Colon, false))
	{
		node->type = this->parse_type_ref(p);
		if (node->type == nullptr)
		{
			_DeletePointer(node);
			return nullptr;
		}
	}

	// = expr
	if (!this->check_token(token_t::Assign, true))
	{
		_DeletePointer(node);
		return nullptr;
	}
	node->value = this->parse_expr(node);
	if (node->value == nullptr)
	{
		_DeletePointer(node);
		return nullptr;
	}

	// ;
	if (!this->check_token(token_t::Semicolon, true))
	{
		_DeletePointer(node);
		return nullptr;
	}

	return node;
}

ast_stmt_continue_t* parser_impl_t::parse_stmt_continue(ast_node_t* p)
{
	ast_stmt_continue_t* node = new ast_stmt_continue_t(p);
	this->next_token();

	// ;
	if (!this->check_token(token_t::Semicolon, true))
	{
		_DeletePointer(node);
		return nullptr;
	}

	return node;
}

ast_stmt_break_t* parser_impl_t::parse_stmt_break(ast_node_t* p)
{
	ast_stmt_break_t* node = new ast_stmt_break_t(p);
	this->next_token();

	// ;
	if (!this->check_token(token_t::Semicolon, true))
	{
		_DeletePointer(node);
		return nullptr;
	}

	return node;
}

ast_stmt_return_t* parser_impl_t::parse_stmt_return(ast_node_t* p)
{
	ast_stmt_return_t* node = new ast_stmt_return_t(p);

	this->next_token(); // ignore 'return'
	if (this->check_token(token_t::Semicolon, false))
	{
		return node;
	}

	node->value = this->parse_expr(node);
	if (node->value == nullptr)
	{
		_DeletePointer(node);
		return nullptr;
	}

	// ;
	if (!this->check_token(token_t::Semicolon, true))
	{
		_DeletePointer(node);
		return nullptr;
	}

	return node;
}

ast_stmt_if_t* parser_impl_t::parse_stmt_if(ast_node_t* p)
{
	ast_stmt_if_t* node = new ast_stmt_if_t(p);

	this->next_token(); // ignore 'if'
	if (!this->check_token(token_t::LRB))
	{
		_DeletePointer(node);
		return nullptr;
	}

	node->cond = this->parse_expr(node);
	if (node->cond == nullptr)
	{
		_DeletePointer(node);
		return nullptr;
	}

	if (!this->check_token(token_t::RRB))
	{
		_DeletePointer(node);
		return nullptr;
	}

	node->branch_true = this->parse_stmt(node);
	if (node->branch_true == nullptr)
	{
		_DeletePointer(node);
		return nullptr;
	}

	if (this->check_token(token_t::Else, false))
	{
		node->branch_false = this->parse_stmt(node);
		if (node->branch_false == nullptr)
		{
			_DeletePointer(node);
			return nullptr;
		}
	}

	return node;
}

ast_stmt_while_t* parser_impl_t::parse_stmt_while(ast_node_t* p)
{
	ast_stmt_while_t* node = new ast_stmt_while_t(p);

	this->next_token();	// ignore 'while'
	if (!this->check_token(token_t::LRB))
	{
		_DeletePointer(node);
		return nullptr;
	}

	node->cond = this->parse_expr(node);
	if (node->cond == nullptr)
	{
		_DeletePointer(node);
		return nullptr;
	}

	if (!this->check_token(token_t::RRB))
	{
		_DeletePointer(node);
		return nullptr;
	}

	node->body = this->parse_stmt(node);
	if (node->body == nullptr)
	{
		_DeletePointer(node);
		return nullptr;
	}

	return node;
}

ast_stmt_for_t* parser_impl_t::parse_stmt_for(ast_node_t* p)
{
	ast_stmt_for_t* node = new ast_stmt_for_t(p);

	this->next_token();	// ignore 'for'

	if (!this->check_token(token_t::LRB))
	{
		_DeletePointer(node);
		return nullptr;
	}

	node->init = this->parse_stmt(node);
	if (node->init == nullptr)
	{
		_DeletePointer(node);
		return nullptr;
	}

	if (!this->check_token(token_t::Semicolon))
	{
		_DeletePointer(node);
		return nullptr;
	}

	node->cond = this->parse_expr(node);
	if (node->cond == nullptr)
	{
		_DeletePointer(node);
		return nullptr;
	}

	if (!this->check_token(token_t::Semicolon))
	{
		_DeletePointer(node);
		return nullptr;
	}

	node->step = this->parse_stmt(node);
	if (node->step == nullptr)
	{
		_DeletePointer(node);
		return nullptr;
	}

	if (!this->check_token(token_t::RRB))
	{
		_DeletePointer(node);
		return nullptr;
	}

	node->body = this->parse_stmt(node);
	if (node->body == nullptr)
	{
		_DeletePointer(node);
		return nullptr;
	}

	return node;
}

ast_stmt_block_t* parser_impl_t::parse_stmt_block(ast_node_t* p)
{
	ast_stmt_block_t* node = new ast_stmt_block_t(p);

	if (!this->check_token(token_t::LCB))
	{
		_DeletePointer(node);
		return nullptr;
	}

	while (!this->check_token(token_t::RCB, false))
	{
		ast_stmt_t* stmt = this->parse_stmt(node);
		if (stmt == nullptr)
		{
			_DeletePointer(node);
			return nullptr;
		}

		node->stmts.push_back(stmt);

		this->check_token(token_t::Semicolon, false);
	}

	return node;
}

ast_stmt_t* parser_impl_t::parse_stmt_assign_or_call(ast_node_t* p)
{
	ast_expr_t* left = this->parse_expr_suffixed(p);
	if (left == nullptr)
		return nullptr;

	if (this->check_token(token_t::Assign, false))
	{
		ast_stmt_assign_t* node = new ast_stmt_assign_t(p);
		node->left = left;
		left->parent = node;

		node->right = this->parse_expr(node);
		if (node->right == nullptr)
		{
			_DeletePointer(node);
			return nullptr;
		}

		// ;
		if (!this->check_token(token_t::Semicolon, true))
		{
			_DeletePointer(node);
			return nullptr;
		}

		return node;
	}
	else if (left->category == ast_node_category_t::expr_func_ref)
	{
		ast_stmt_call_t* node = new ast_stmt_call_t(p);
		node->expr = dynamic_cast<ast_expr_func_ref_t*>(left);
		left->parent = node;

		// ;
		if (!this->check_token(token_t::Semicolon, true))
		{
			_DeletePointer(node);
			return nullptr;
		}

		return node;
	}
	else
	{
		this->error_token_unexpected();
		_DeletePointer(left);
		return nullptr;
	}
}

void parser_impl_t::next_token()
{
	return m_scanner->next_token();
}

token_t& parser_impl_t::token()
{
	return m_scanner->token();
}

token_t& parser_impl_t::look_ahead_token()
{
	return m_scanner->look_ahead_token();
}

bool parser_impl_t::check_token(const token_t::token_type& tokenType, bool required, bool movenext)
{
	if (m_scanner->token().type != tokenType)
	{
		if (required)
		{
			this->error_token_unexpected();
		}
		return false;
	}
	if (movenext)
	{
		m_scanner->next_token();
	}
	return true;
}

ast_unary_oper_t parser_impl_t::check_unary_oper(bool required, bool movenext)
{
	ast_unary_oper_t oper = ast_unary_oper_t::Unknown;
	switch (m_scanner->token().type)
	{
	case token_t::Add: oper = ast_unary_oper_t::Pos; break;
	case token_t::Sub: oper = ast_unary_oper_t::Neg; break;
	case token_t::Not: oper = ast_unary_oper_t::Not; break;
	case token_t::At: oper = ast_unary_oper_t::TypeOf; break;
	case token_t::Pound: oper = ast_unary_oper_t::SizeOf; break;
	default: break;
	}
	if (oper == ast_unary_oper_t::Unknown)
	{
		if (required)
		{
			this->error_token_unexpected();
		}
		return oper;
	}
	else if (movenext)
	{
		m_scanner->next_token();
	}
	return oper;
}

ast_binary_oper_t parser_impl_t::check_binary_oper(int priority, bool required, bool movenext)
{
	ast_binary_oper_t oper = ast_binary_oper_t::Unknown;
	switch (m_scanner->token().type)
	{
	case token_t::Or2: oper = ast_binary_oper_t::Or; break;
	case token_t::And2: oper = ast_binary_oper_t::And; break;
	case token_t::Is: oper = ast_binary_oper_t::Is; break;
	case token_t::Equal: oper = ast_binary_oper_t::Equal; break;
	case token_t::Greater: oper = ast_binary_oper_t::Greater; break;
	case token_t::Less: oper = ast_binary_oper_t::Less; break;
	case token_t::GEqual: oper = ast_binary_oper_t::GEqual; break;
	case token_t::LEqual: oper = ast_binary_oper_t::LEqual; break;
	case token_t::NEqual: oper = ast_binary_oper_t::NEqual; break;
	case token_t::Add: oper = ast_binary_oper_t::Add; break;
	case token_t::Sub: oper = ast_binary_oper_t::Sub; break;
	case token_t::Mul: oper = ast_binary_oper_t::Mul; break;
	case token_t::Div: oper = ast_binary_oper_t::Div; break;
	case token_t::Mod: oper = ast_binary_oper_t::Mod; break;
	case token_t::And: oper = ast_binary_oper_t::BitAnd; break;
	case token_t::Or: oper = ast_binary_oper_t::BitOr; break;
	case token_t::Xor: oper = ast_binary_oper_t::BitXor; break;
	case token_t::ShiftL: oper = ast_binary_oper_t::ShiftL; break;
	case token_t::ShiftR: oper = ast_binary_oper_t::ShiftR; break;
	case token_t::As: oper = ast_binary_oper_t::As; break;
	default: break;
	}
	if ((int)oper / 100 != priority)
	{
		oper = ast_binary_oper_t::Unknown;
	}
	if (oper == ast_binary_oper_t::Unknown)
	{
		if (required)
		{
			this->error_token_unexpected();
		}
		return oper;
	}
	else if (movenext)
	{
		m_scanner->next_token();
	}
	return oper;
}

const String& parser_impl_t::error()
{
	return m_error;
}

void parser_impl_t::error(const char* fmt, ...)
{
	String message;
	_FormatVA(message, fmt);
	m_error = String::format("%s at %d, %d.\n", message.cstr(), m_scanner->line(), m_scanner->column());
}

void parser_impl_t::error_token_unexpected()
{
	token_t& token = m_scanner->token();
	const char* value = token.value.cstr();
	if (token.type == token_t::Eos)
	{
		this->error("unexpected eos");
	}
	else
	{
		this->error("unexpected token '%s'", value);
	}
}

parser_t::parser_t()
	: impl(new parser_impl_t())
{}

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
