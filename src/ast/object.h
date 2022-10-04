#ifndef _EOKAS_AST_OBJECT_H_
#define _EOKAS_AST_OBJECT_H_

#include "header.h"
#include "nodes.h"

namespace eokas
{
	class ast_scope_t
	{
		ast_node_func_def_t* func;
		ast_scope_t* parent;
		std::list<ast_scope_t*> children;
		std::map<String, ast_node_symbol_def_t*> symbols;

	public:
		ast_scope_t(ast_node_func_def_t* func, ast_scope_t* parent)
			: func(func), parent(parent), children(), symbols()
		{ }

		virtual ~ast_scope_t()
		{
			this->func = nullptr;
			this->parent = nullptr;
			_DeleteList(this->children);
			this->symbols.clear();
		}
		
		ast_scope_t* add_child(ast_node_func_def_t* func = nullptr)
		{
			func = func != nullptr ? func : this->func;
			auto* child = new ast_scope_t(func, this);
			this->children.push_back(child);
			return child;
		}
		
		ast_node_symbol_def_t* get_symbol(const String& name, bool lookup = true)
		{
			if(!lookup)
			{
				auto iter = this->symbols.find(name);
				if(iter != this->symbols.end())
					return iter->second;
				return nullptr;
			}

			for (auto scope = this; scope != nullptr; scope = scope->parent)
			{
				auto iter = scope->symbols.find(name);
				if(iter != scope->symbols.end())
					return iter->second;
			}

			return nullptr;
		}

		void set_symbol(const String& name, ast_node_symbol_def_t* symbol)
		{
			this->symbols.insert(std::make_pair(name, symbol));
		}
	};
}

#endif //_EOKAS_AST_OBJECT_H_
