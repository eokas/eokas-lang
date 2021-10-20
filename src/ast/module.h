
#ifndef _EOKAS_AST_MODULE_H_
#define _EOKAS_AST_MODULE_H_

#include "header.h"

namespace eokas
{
	class ast_module_t
	{
	public:
		explicit ast_module_t();
		virtual ~ast_module_t();
		
	public:
		[[nodiscard]] ast_factory_t* get_factory() const;
		[[nodiscard]] ast_expr_func_def_t* get_entry() const;
		[[nodiscard]] ast_expr_func_def_t* get_func() const;
		[[nodiscard]] ast_scope_t* get_scope() const;
		
	private:
		ast_factory_t* factory;
		ast_expr_func_def_t* entry;
		ast_expr_func_def_t* func;
		ast_scope_t* scope;
	};
}

#endif //_EOKAS_AST_MODULE_H_
