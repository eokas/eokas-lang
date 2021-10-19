
#ifndef _EOKAS_AST_OBJECT_H_
#define _EOKAS_AST_OBJECT_H_

#include "header.h"

namespace eokas
{
	class ast_type_t
	{
	private:
		String name;
	};
	
	class ast_expr_t
	{
	public:
		explicit ast_expr_t(ast_type_t* type);
		virtual ~ast_expr_t();
		
		ast_type_t* getType();
		
	private:
		ast_type_t* type;
	};
	
	class ast_constant_t :public ast_expr_t
	{ };
	
	class ast_int_t :public ast_constant_t
	{ };
	
	class ast_float_t :public ast_constant_t
	{ };
	
	class ast_bool_t :public ast_constant_t
	{ };
	
	class ast_string_t :public ast_constant_t
	{ };
	
	class ast_object_t :public ast_constant_t
	{ };
	
	class ast_array_t :public ast_constant_t
	{ };
	
	class ast_func_t :public ast_constant_t
	{ };
	
	class ast_symbol_t :public ast_expr_t
	{
	public:
		ast_symbol_t(const String& name, ast_type_t* type, ast_expr_t* value);
		~ast_symbol_t() override;
		
		const String& getName() const;
		const ast_type_t* getType() const;
		const ast_expr_t* getValue() const;
	
	private:
		String name;
		ast_expr_t* value;
	};
	
	class ast_scope_t
	{
	public:
		explicit ast_scope_t(ast_scope_t* parent);
		virtual ~ast_scope_t();
		
		ast_scope_t* add_child();
		
		ast_type_t* get_type(const String& name, bool lookup = true);
		void set_type(const String& name, ast_type_t* type);
		
		ast_symbol_t* get_symbol(const String& name, bool lookup = true);
		void set_symbol(const String& name, ast_symbol_t* symbol);
		
	private:
		ast_scope_t* parent;
		std::list<ast_scope_t*> children;
		std::map<String, ast_type_t*> types;
		std::map<String, ast_symbol_t*> symbols;
	};
}

#endif //_EOKAS_AST_OBJECT_H_
