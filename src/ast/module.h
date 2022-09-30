#ifndef _EOKAS_AST_MODULE_H_
#define _EOKAS_AST_MODULE_H_

#include "header.h"
#include "nodes.h"

namespace eokas
{
	class ast_module_t
	{
		ast_factory_t* factory;
		ast_expr_func_def_t* entry;
		ast_expr_func_def_t* func;
		ast_scope_t* scope;

	public:
		explicit ast_module_t()
			: factory(new ast_factory_t()), entry(nullptr), func(nullptr), scope(nullptr)
		{
			this->entry = factory->create_expr_func_def(nullptr);
			this->func = this->entry;
			this->scope = new ast_scope_t(this->func, nullptr);
		}

		virtual ~ast_module_t()
		{
			_DeletePointer(this->factory);
			this->entry = nullptr;
			this->func = nullptr;
			_DeletePointer(this->scope);
		}

		ast_factory_t* get_factory() const
		{
			return this->factory;
		}

		ast_expr_func_def_t* get_entry() const
		{
			return this->entry;
		}

		ast_expr_func_def_t* get_func() const
		{
			return this->func;
		}

		ast_scope_t* get_scope() const
		{
			return this->scope;
		}
	};
}

#endif //_EOKAS_AST_MODULE_H_
