#ifndef _EOKAS_LLVM_SCOPE_H_
#define _EOKAS_LLVM_SCOPE_H_

#include "./header.h"

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
	
	struct llvm_scope_t
	{
		llvm_scope_t* parent;
		llvm_function_t* func;
		std::vector<llvm_scope_t*> children;
		
		table_t<llvm_type_t> types;
		table_t<llvm_value_t> values;
		
		llvm_scope_t(llvm_scope_t* parent, llvm_function_t* func);
		virtual ~llvm_scope_t();
		
		llvm_scope_t* add_child(llvm_function_t* f = nullptr);
		
		bool add_type(const String& name, llvm_type_t* type);
		llvm_type_t* get_type(const String& name, bool lookup);
		
		bool add_value(const String& name, llvm_value_t* value);
		llvm_value_t* get_value(const String& name, bool lookup);
	};
}

#endif//_EOKAS_LLVM_SCOPE_H_
