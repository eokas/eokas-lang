#ifndef _EOKAS_LLVM_OBJECT_H_
#define _EOKAS_LLVM_OBJECT_H_

#include "header.h"

_BeginNamespace(eokas)
	
	struct llvm_expr_t
	{
		llvm::Value* value;
		llvm::Type* type;
		
		explicit llvm_expr_t(llvm::Value* value, llvm::Type* type = nullptr);
		virtual ~llvm_expr_t();
	};

	enum class llvm_type_category_t
	{
		Basic,
		Struct
	};

	struct llvm_type_t
	{
		llvm_type_category_t category = llvm_type_category_t::Basic;
		String name = "";
		llvm::Type* type = nullptr;
		
		virtual ~llvm_type_t()
		{
			this->type = nullptr;
		}
	};
	
	struct llvm_struct_t :public llvm_type_t
	{
		struct member_t
		{
			String name = "";
			llvm_type_t* type = {};
		};
		
		llvm_struct_t* base = nullptr;
		std::vector<member_t*> members = {};
		
		~llvm_struct_t() override
		{
			_DeleteList(this->members);
		}
		
		member_t* addMember(const String& name, llvm_type_t* type)
		{
			auto* m = new member_t();
			m->name = name;
			m->type = type;
			this->members.push_back(m);
			return m;
		}
		
		[[nodiscard]] member_t* getMember(const String& name) const
		{
			for(auto& m : this->members)
			{
				if(m->name == name)
					return m;
			}
			return nullptr;
		}
		
		[[nodiscard]] size_t getMemberIndex(const String& name) const
		{
			for(size_t index = 0; index < this->members.size(); index++)
			{
				if(this->members.at(index)->name == name)
					return index;
			}
			return -1;
		}
		
		[[nodiscard]] member_t* getMember(u32_t index) const
		{
			if(index >= this->members.size())
				return nullptr;
			return this->members.at(index);
		}
	};

	struct llvm_scope_t
	{
		llvm_scope_t* parent;
        llvm::Function* func;
		std::vector<llvm_scope_t*> children;

		std::map<String, llvm_expr_t*> symbols;
		std::map<String, llvm_type_t*> types;
		
		explicit llvm_scope_t(llvm_scope_t* parent, llvm::Function* func);
		virtual ~llvm_scope_t();
		
		llvm_scope_t* addChild(llvm::Function* f = nullptr);
		llvm_expr_t* getSymbol(const String& name, bool lookup);
		llvm_type_t* getType(const String& name, bool lookup);
	};
	
_EndNamespace(eokas)

#endif//_EOKAS_LLVM_OBJECT_H_
