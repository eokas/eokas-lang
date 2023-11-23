
#ifndef _EOKAS_OMIS_MODEL_H_
#define _EOKAS_OMIS_MODEL_H_

#include "./header.h"

namespace eokas {
    enum class omis_typeid_t {
        UNKNOWN,
        I8, I16, I32, I64,
        U8, U16, U32, U64,
        F32, F64,
        BOOL,
        FUNC,
        STRUCT,
        STRING,

    };

    class omis_type_t {
    public:
        omis_type_t(omis_module_t* module, omis_handle_t handle);
        virtual ~omis_type_t();

        omis_module_t* get_module();
        omis_handle_t get_handle();
        omis_value_t* get_default_value();

    protected:
        omis_module_t* module;
        omis_handle_t handle;
        omis_value_t* default_value;
    };

    class omis_struct_t :public omis_type_t {
    public:
        struct member_t {
            String name;
            omis_type_t* type;
            omis_value_t* value;
        };

        omis_struct_t(omis_module_t* module, omis_typeid_t id, omis_handle_t handle);
        virtual ~omis_struct_t();

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

    protected:
        omis_module_t* module;
        omis_type_t* type;
        omis_handle_t handle;
    };

    class omis_func_t :public omis_value_t {
    public:
        omis_func_t(omis_module_t* module, omis_type_t* type, omis_handle_t handle);
        virtual ~omis_func_t();

        omis_value_t* create_block(const String& name);
        void activate_block(omis_value_t* block);

        omis_value_t* load(omis_value_t* ptr);
        void store(omis_type_t* ptr, omis_value_t* val);

        omis_value_t* neg(omis_value_t* a);
        omis_value_t* add(omis_value_t* a, omis_value_t* b);
        omis_value_t* sub(omis_value_t* a, omis_value_t* b);
        omis_value_t* mul(omis_value_t* a, omis_value_t* b);
        omis_value_t* div(omis_value_t* a, omis_value_t* b);
        omis_value_t* mod(omis_value_t* a, omis_value_t* b);
        omis_value_t* cmp(omis_value_t* a, omis_value_t* b);
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
        omis_value_t* phi(omis_type_t* type, const std::map<omis_value_t*, omis_value_t*> incommings);

        omis_value_t* create_local_symbol(const String& name, omis_type_t* type, omis_value_t* value);
        void ensure_tail_ret();

    protected:
    };

    class omis_module_t {
    public:
        omis_module_t(const String& name, omis_bridge_t* bridge);
        virtual ~omis_module_t();

        omis_bridge_t* get_bridge();

        omis_scope_t* get_scope();
        omis_scope_t* push_scope(omis_func_t* func = nullptr);
        omis_scope_t* pop_scope();

        bool using_module(omis_module_t* other);

        omis_type_t* create_type(omis_handle_t handle);
        bool equals_type(omis_type_t* a, omis_type_t* b);
        bool can_losslessly_bitcast(omis_type_t* a, omis_type_t* b);
        omis_type_t* type_void();
        omis_type_t* type_i8();
        omis_type_t* type_i16();
        omis_type_t* type_i32();
        omis_type_t* type_f32();
        omis_type_t* type_i64();
        omis_type_t* type_f64();
        omis_type_t* type_bool();


        omis_value_t* create_value(omis_type_t* type, omis_handle_t handle);
        omis_func_t* create_func(const String& name, omis_type_t* ret, const std::vector<omis_type_t*>& args);
        omis_value_t* constant_integer(u64_t val, u32_t bits);
        omis_value_t* constant_float(f64_t val);
        omis_value_t* constant_bool(bool val);
        omis_value_t* constant_string(const String& val);

    protected:
        String name;
        omis_bridge_t* bridge;
        omis_scope_t* root;
        omis_scope_t* scope;
        std::vector<omis_module_t*> usings;
        std::vector<omis_type_t*> types;
        std::vector<omis_value_t*> values;

        omis_type_t* m_type_i32;
    };
}

#endif //_EOKAS_OMIS_MODEL_H_
