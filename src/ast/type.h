
#ifndef _EOKAS_AST_TYPE_H_
#define _EOKAS_AST_TYPE_H_

#include "header.h"

namespace eokas
{
	struct ast_type_t : public ast_node_t
	{
		ast_type_t(ast_node_category_t category, ast_node_t* parent);
	};
	
	struct ast_type_ref_t : public ast_type_t
	{
		String name;
		
		explicit ast_type_ref_t(ast_node_t* parent);
	};
	
	struct ast_type_array_t : public ast_type_t
	{
		ast_type_t* elementType;
		u32_t length;
		
		explicit ast_type_array_t(ast_node_t* parent);
	};
	
	struct ast_type_generic_t : public ast_type_t
	{
		String name;
		std::vector<ast_type_t*> args;
		
		explicit ast_type_generic_t(ast_node_t* parent);
	};
}

#endif //_EOKAS_AST_TYPE_H_