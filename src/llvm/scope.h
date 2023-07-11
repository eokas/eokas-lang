#ifndef _EOKAS_LLVM_SCOPE_H_
#define _EOKAS_LLVM_SCOPE_H_

#include "./header.h"
#include "./builder.h"
#include <llvm/IR/IRBuilder.h>

namespace eokas
{
	template<typename T, bool gc = true>
	struct table_t
	{
		std::map<String, T*> table;
		
		explicit table_t() : table()
		{
		}
		
		~table_t()
		{
			if(gc && !this->table.empty())
			{
				_DeleteMap(this->table);
			}
			this->table.clear();
		}
		
		bool add(const String& name, T* object)
		{
			if(this->table.find(name) != this->table.end())
				return false;
			this->table.insert(std::make_pair(name, object));
			return true;
		}
		
		T* get(const String& name)
		{
			auto iter = this->table.find(name);
			if(iter == this->table.end())
				return nullptr;
			return iter->second;
		}
	};
	
	struct llvm_symbol_t
	{
		struct llvm_scope_t* scope = nullptr;
		llvm::Type* type = nullptr;
		llvm::Value* value = nullptr;
	};
	
	struct llvm_scope_t
	{
		llvm_scope_t* parent;
		llvm_func_builder_t* func;
		std::vector<llvm_scope_t*> children;
		
		table_t<llvm_symbol_t> symbols;
		table_t<llvm_type_builder_t> types;
		
		llvm_scope_t(llvm_scope_t* parent, llvm_func_builder_t* func);
		virtual ~llvm_scope_t();
		
		llvm_scope_t* addChild(llvm_func_builder_t* f = nullptr);
		
		bool addSymbol(const String& name, llvm::Value* expr);
		bool addSymbol(const String& name, llvm::Type* type);
		llvm_symbol_t* getSymbol(const String& name, bool lookup);
		
		bool addType(const String& name, llvm_type_builder_t* type);
		llvm_type_builder_t* getType(const String& name, bool lookup);
	};
}

#endif//_EOKAS_LLVM_SCOPE_H_
