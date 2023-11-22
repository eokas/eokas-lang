
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
        omis_type_t(omis_module_t* module, omis_typeid_t id, omis_handle_t handle);
        virtual ~omis_type_t();

        omis_module_t* get_module();
        omis_typeid_t get_typeid();
        omis_handle_t get_handle();
        omis_value_t* get_default_value();

    protected:
        omis_module_t* module;
        omis_typeid_t id;
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

        omis_value_t* load(omis_value_t* ptr);
        void store(omis_type_t* ptr, omis_value_t* val);

        omis_value_t* neg(omis_value_t* a);
        omis_value_t* add(omis_value_t* a, omis_value_t* b);
        omis_value_t* sub(omis_value_t* a, omis_value_t* b);
        omis_value_t* mul(omis_value_t* a, omis_value_t* b);
        omis_value_t* div(omis_value_t* a, omis_value_t* b);
        omis_value_t* mod(omis_value_t* a, omis_value_t* b);
        omis_value_t* cmp(omis_value_t* a, omis_value_t* b);
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
        omis_value_t* jump_break();
        omis_value_t* jump_continue();
        omis_value_t* call(omis_func_t* func, const std::vector<omis_value_t*> args);
        void ret(omis_value_t* value);

    protected:
    };

    class omis_module_t {
    public:
        omis_module_t(const String& name, omis_bridge_t* bridge);
        virtual ~omis_module_t();

        omis_bridge_t* get_bridge();
        omis_scope_t* get_scope();
        bool using_module(omis_module_t* other);

        omis_type_t* create_type(omis_typeid_t id, omis_handle_t handle);
        omis_struct_t* create_struct()
        omis_value_t* create_value(omis_type_t* type, omis_handle_t handle);
        omis_func_t* create_func(const String& name, omis_type_t* ret, const std::vector<omis_type_t*>& args);

    protected:
        String name;
        omis_bridge_t* bridge;
        omis_scope_t* scope;
        std::vector<omis_module_t*> usings;
        std::vector<omis_type_t*> types;
        std::vector<omis_value_t*> values;
    };
}

#endif //_EOKAS_OMIS_MODEL_H_
