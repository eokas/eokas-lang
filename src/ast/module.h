
#ifndef _EOKAS_AST_MODULE_H_
#define _EOKAS_AST_MODULE_H_

#include "header.h"

namespace eokas
{
	class ast_module_t
	{
	public:
		ast_module_t();
		virtual ~ast_module_t();
		
	private:
		std::vector<ast_node_t*> nodes;
		ast_func_t* func;
		ast_scope_t* scope;
	};
}

#endif //_EOKAS_AST_MODULE_H_
