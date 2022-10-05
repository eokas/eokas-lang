#ifndef _EOKAS_AST_FACTORY_H_
#define _EOKAS_AST_FACTORY_H_

#include "header.h"
#include "nodes.h"

#include <concepts>

namespace eokas
{
	template<typename T>
	concept ast_concept_node = std::is_base_of<ast_node_t, T>::value;
	
	class ast_factory_t
	{
		std::vector<ast_node_t*> nodes = {};

	public:
		virtual ~ast_factory_t()
		{
			this->clear();
		}
		
		void clear()
		{
			_DeleteList(nodes);
		}
		
		template<ast_concept_node Node>
		Node* create(ast_node_t* parent)
		{
			auto* node = new Node(parent);
			this->nodes.push_back(node);
			return node;
		}
	};
}

#endif //_EOKAS_AST_FACTORY_H_
