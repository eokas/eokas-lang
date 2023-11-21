
#ifndef _EOKAS_OMIS_MODEL_H_
#define _EOKAS_OMIS_MODEL_H_

#include "./header.h"

namespace eokas {
    struct omis_type_t {
        virtual omis_module_t* get_module() = 0;
        virtual void* get_handle() = 0;
        virtual void* get_default_value() = 0;
        virtual bool equals(omis_type_t* other) = 0;
    };

    struct omis_type_struct_t :public omis_type_t {
        struct member_t {
            String name;
            omis_type_t* type;
            omis_value_t* value;
        };

        virtual void begin() = 0;
        virtual void body() = 0;
        virtual void end() = 0;

        void extends(omis_type_struct_t* other);

        member_t* add_member(const String& name, omis_type_t* type, omis_value_t* value);
        member_t* add_member(const String& name, omis_value_t* value);
        member_t* add_member(member_t* other);
        member_t* get_member(const String& name);
        member_t* get_member(size_t index);
        size_t get_member_index(const String& name);
    };

    struct omis_value_t {
        virtual omis_module_t* get_module() = 0;
        virtual void* get_handle() = 0;
        virtual omis_type_t* get_type() = 0;
        virtual bool equals(omis_value_t* other) = 0;
    };

    struct omis_func_t :public omis_value_t {

    };

    struct omis_module_t {
        virtual bool use_module(omis_module_t* other) = 0;

        virtual omis_scope_t* get_scope() = 0;

        virtual omis_type_t* create_type(void* handle) = 0;

        virtual omis_value_t* create_value(void* handle) = 0;
        virtual omis_func_t* create_func(const String& name, omis_type_t* ret, const std::vector<omis_type_t*> args);

    };
}

#endif //_EOKAS_OMIS_MODEL_H_
