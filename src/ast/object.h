
#ifndef _EOKAS_AST_OBJECT_H_
#define _EOKAS_AST_OBJECT_H_

#include "header.h"

namespace eokas
{
	class ast_scope_t
	{
	public:
		explicit ast_scope_t(ast_scope_t* parent);
		virtual ~ast_scope_t();
		
		ast_scope_t* add_child();
		
		ast_type_t* get_type(const String& name, bool lookup = true);
		void set_type(const String& name, ast_type_t* type);
		
		ast_expr_symbol_def_t* get_symbol(const String& name, bool lookup = true);
		void set_symbol(const String& name, ast_expr_symbol_def_t* symbol);
		
	private:
		ast_scope_t* parent;
		std::list<ast_scope_t*> children;
		std::map<String, ast_type_t*> types;
		std::map<String, ast_expr_symbol_def_t*> symbols;
	};
}

#endif //_EOKAS_AST_OBJECT_H_
