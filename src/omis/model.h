
#ifndef _EOKAS_OMIS_MODEL_H_
#define _EOKAS_OMIS_MODEL_H_

#include "./header.h"

namespace eokas {
    template<typename T, bool gc = true>
    struct omis_table_t {
    	using container_t = std::map<String, T*>;
    	using iterator_t = typename container_t::iterator;
    	using const_iterator_t = typename container_t::const_iterator;

    	container_t table;

    	explicit omis_table_t() : table() {
    	}

    	~omis_table_t() {
    		if (gc && !this->table.empty()) {
    			_DeleteMap(this->table);
    		}
    		this->table.clear();
    	}

    	iterator_t begin() {
    		return table.begin();
    	}

    	const_iterator_t begin() const {
    		return table.begin();
    	}

    	iterator_t end() {
    		return table.end();
    	}

    	const_iterator_t end() const {
    		return table.end();
    	}

    	bool add(const String &name, T *object) {
    		if (this->table.find(name) != this->table.end())
    			return false;
    		this->table.insert(std::make_pair(name, object));
    		return true;
    	}

    	T *get(const String &name) {
    		auto iter = this->table.find(name);
    		if (iter == this->table.end())
    			return nullptr;
    		return iter->second;
    	}

    	T *get(const std::function<bool(const String&, const T&)>& predicate) {
    		for(auto& pair : this->table) {
    			if(predicate(pair.first, *pair.second)) {
    				return pair.second;
    			}
    		}
    		return nullptr;
    	}
    };

    struct omis_value_symbol_t
    {
        String name = "";
        omis_value_t* value = nullptr;
        omis_scope_t* scope = nullptr;
    };

    struct omis_type_symbol_t
    {
        String name = "";
        omis_type_t* type = nullptr;
    };

    struct omis_scope_t
    {
        omis_scope_t* parent;
        omis_value_t* func;
        std::vector<omis_scope_t*> children;

        omis_table_t<omis_type_symbol_t> types;
        omis_table_t<omis_value_symbol_t> values;

        omis_scope_t(omis_scope_t* parent, omis_value_t* func);
        virtual ~omis_scope_t();

        omis_scope_t* add_child(omis_value_t* f = nullptr);

        bool add_type_symbol(const String& name, omis_type_t* type);
        omis_type_symbol_t* get_type_symbol(const String& name, bool lookup);
        omis_type_symbol_t* get_type_symbol(omis_lambda_predicate_t<omis_type_symbol_t> predicate, bool lookup);

        bool add_value_symbol(const String& name, omis_value_t* value);
        omis_value_symbol_t* get_value_symbol(const String& name, bool lookup);
        omis_value_symbol_t* get_value_symbol(omis_lambda_predicate_t<omis_value_symbol_t> predicate, bool lookup);
    };

    class omis_module_t {
    public:
        omis_module_t(omis_context_t* context, const String& name);
        virtual ~omis_module_t();

        virtual bool main();

    	omis_context_t* get_context() const;
        omis_bridge_t* get_bridge() const;
        const String& get_name() const;
        omis_handle_t get_handle() const;
        String dump() const;

        bool using_module(const String& name);

        omis_scope_t* get_scope();
        omis_scope_t* push_scope(omis_value_t* func = nullptr);
        omis_scope_t* pop_scope();

    	bool add_type_symbol(const String& name, omis_type_t* type);
        omis_type_symbol_t* get_type_symbol(const String& name, bool lookup = true);
    	bool add_value_symbol(const String& name, omis_value_t* type);
        omis_value_symbol_t* get_value_symbol(const String& name, bool lookup = true);

        omis_type_t* type(omis_handle_t handle);
        omis_type_t* type_void();
        omis_type_t* type_i8();
        omis_type_t* type_i16();
        omis_type_t* type_i32();
        omis_type_t* type_f32();
        omis_type_t* type_i64();
        omis_type_t* type_f64();
        omis_type_t* type_bool();
        omis_type_t* type_bytes();
        omis_type_t* type_pointer(omis_type_t* type);
        omis_type_t* type_func(omis_type_t* ret, const std::vector<omis_type_t*>& args, bool varg);
		String get_type_name(omis_type_t* type);
        omis_value_t* get_type_size(omis_type_t* type);
        bool can_losslessly_bitcast(omis_type_t* a, omis_type_t* b);

        omis_value_t* value(omis_type_t* type, omis_handle_t handle);
        omis_value_t* value(omis_handle_t handle);
        omis_value_t* value_integer(u64_t val, u32_t bits);
        omis_value_t* value_float(f64_t val);
        omis_value_t* value_bool(bool val);
        omis_value_t* value_string(const String& val);
        omis_value_t* value_func(const String& name, omis_type_t* ret, const std::vector<omis_type_t*>& args, bool varg);

		omis_type_t* get_func_ret_type(omis_value_t* func);
		uint32_t get_func_arg_count(omis_value_t* func);
		omis_type_t* get_func_arg_type(omis_value_t* func, uint32_t index);
		omis_value_t* get_func_arg_value(omis_value_t* func, uint32_t index);
		
        bool equals_type(omis_type_t* a, omis_type_t* b);
        bool equals_value(omis_value_t* a, omis_value_t* b);
		
		omis_value_t* create_block(const String& name);
		omis_value_t* get_active_block();
		void set_active_block(omis_value_t* block);
		omis_value_t* get_block_tail(omis_value_t* block);
		bool is_terminator_ins(omis_value_t* ins = nullptr);
		
		omis_value_t* alloc(const String& name, omis_type_t* type, omis_value_t* value = nullptr);
		omis_value_t* load(omis_value_t* ptr);
		omis_value_t* store(omis_value_t* ptr, omis_value_t* val);
		omis_value_t* neg(omis_value_t* a);
		omis_value_t* add(omis_value_t* a, omis_value_t* b);
		omis_value_t* sub(omis_value_t* a, omis_value_t* b);
		omis_value_t* mul(omis_value_t* a, omis_value_t* b);
		omis_value_t* div(omis_value_t* a, omis_value_t* b);
		omis_value_t* mod(omis_value_t* a, omis_value_t* b);
		omis_value_t* eq(omis_value_t* a, omis_value_t* b);
		omis_value_t* ne(omis_value_t* a, omis_value_t* b);
		omis_value_t* gt(omis_value_t* a, omis_value_t* b);
		omis_value_t* ge(omis_value_t* a, omis_value_t* b);
		omis_value_t* lt(omis_value_t* a, omis_value_t* b);
		omis_value_t* le(omis_value_t* a, omis_value_t* b);
		omis_value_t* l_not(omis_value_t* a);
		omis_value_t* l_and(omis_value_t* a, omis_value_t* b);
		omis_value_t* l_or(omis_value_t* a, omis_value_t* b);
		omis_value_t* b_flip(omis_value_t* a);
		omis_value_t* b_and(omis_value_t* a, omis_value_t* b);
		omis_value_t* b_or(omis_value_t* a, omis_value_t* b);
		omis_value_t* b_xor(omis_value_t* a, omis_value_t* b);
		omis_value_t* b_shl(omis_value_t* a, omis_value_t* b);
		omis_value_t* b_shr(omis_value_t* a, omis_value_t* b);
		omis_value_t* jump(omis_value_t* pos);
		omis_value_t* jump_cond(omis_value_t* cond, omis_value_t* branch_true, omis_value_t* branch_false);
		omis_value_t* phi(omis_type_t* type, const std::map<omis_value_t*, omis_value_t*>& incomings);
		omis_value_t* call(omis_value_t* func, const std::vector<omis_value_t*>& args);
		omis_value_t* call(const String& func, const std::vector<omis_value_t*>& args);
		omis_value_t* ret(omis_value_t* value = nullptr);
		omis_value_t* bitcast(omis_value_t* value, omis_type_t* type);
		
		omis_value_t* get_ptr_val(omis_value_t* val);
		omis_value_t* get_ptr_ref(omis_value_t* val);
		omis_value_t* make(omis_type_t* type);
		omis_value_t* make(omis_type_t* type, omis_value_t* count);
		omis_value_t* drop(omis_value_t* ptr);
		
		omis_value_t* expr_branch(const omis_lambda_expr_t& lambda_cond, const omis_lambda_expr_t& lambda_true, const omis_lambda_expr_t& lambda_false);
		
		bool stmt_block(const std::optional<omis_lambda_stmt_t>& lambda_body);
		bool stmt_symbol_def(const String& name, const std::optional<omis_lambda_type_t>& lambda_type, const omis_lambda_expr_t& lambda_expr);
		bool stmt_assign(const omis_lambda_expr_t& lambda_left, const omis_lambda_expr_t& lambda_right);
		bool stmt_return(const std::optional<omis_lambda_expr_t>& lambda_expr);
		bool stmt_branch(const omis_lambda_expr_t& lambda_cond,
						 const omis_lambda_stmt_t& lambda_true,
						 const omis_lambda_stmt_t& lambda_false);
		bool stmt_loop(const omis_lambda_stmt_t& lambda_init,
					   const omis_lambda_expr_t& lambda_cond,
					   const omis_lambda_stmt_t& lambda_step,
					   const omis_lambda_stmt_t& lambda_body);
		bool stmt_break();
		bool stmt_continue();
		void stmt_ensure_tail_ret(omis_value_t* func);
		
	protected:
    	omis_context_t* context;
        omis_bridge_t* bridge;
        String name;
        omis_handle_t handle;
        omis_scope_t* root;
    	omis_scope_t* scope;

    	std::map<String, omis_module_t*> dependencies;
    	omis_scope_t* imports;
    	omis_scope_t* exports;

        std::map<omis_handle_t, omis_type_t*> types;
        std::map<omis_handle_t, omis_value_t*> values;
		omis_value_t* break_point;
		omis_value_t* continue_point;
    };

    class omis_type_t {
    public:
        omis_type_t(omis_module_t* module, omis_handle_t handle);
        virtual ~omis_type_t();

        omis_module_t* get_module();
        omis_handle_t get_handle();
        omis_value_t* get_default_value();
		
		bool is_type_void();
		bool is_type_i8();
		bool is_type_i16();
		bool is_type_i32();
		bool is_type_i64();
		bool is_type_f32();
		bool is_type_f64();
		bool is_type_bool();
		bool is_type_bytes();
        bool is_type_func();
        bool is_type_array();
        bool is_type_struct();

        omis_type_t* get_pointer_type();

    protected:
        omis_module_t* module;
        omis_handle_t handle;
        omis_value_t* default_value;
		String name;
    };

    class omis_struct_t :public omis_type_t {
    public:
        struct member_t {
            String name;
            omis_type_t* type;
            omis_value_t* value;
        };

        omis_struct_t(omis_module_t* module, omis_handle_t handle);
        virtual ~omis_struct_t();

        virtual bool main();

        bool extends(const String& base);
        bool extends(omis_struct_t* base);

        member_t* add_member(const String& name, omis_type_t* type, omis_value_t* value = nullptr);
        member_t* add_member(const String& name, omis_value_t* value);
        member_t* add_member(member_t* other);
        member_t* get_member(const String& name);
        member_t* get_member(size_t index);
        size_t get_member_index(const String& name);

    protected:
        std::vector<member_t> members;
    };

    class omis_value_t {
    public:
        omis_value_t(omis_module_t* module, omis_type_t* type, omis_handle_t handle);
        virtual ~omis_value_t();

        omis_module_t* get_module();
        omis_type_t* get_type();
        omis_handle_t get_handle();
        void set_name(const String& name);

    protected:
        omis_module_t* module;
        omis_type_t* type;
        omis_handle_t handle;
    };
}

#endif //_EOKAS_OMIS_MODEL_H_
