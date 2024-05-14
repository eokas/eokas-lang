
#ifndef _EOKAS_LLVM_MODULE_CORE_H_
#define _EOKAS_LLVM_MODULE_CORE_H_

#include "./model.h"

namespace eokas {
    struct omis_type_object_t :public omis_struct_t {
        omis_type_object_t(omis_module_t* module);

        virtual bool main() override;

        omis_value_t* typeinfo();
        omis_value_t* string();
        omis_value_t* hash();
    };

    struct omis_type_typeinfo_t : omis_type_object_t {
        omis_type_typeinfo_t(omis_module_t* module);

        virtual bool main() override;

        omis_value_t* name();
        omis_value_t* base();
        omis_value_t* fields();
        omis_value_t* methods();
        omis_value_t* make();
    };

    struct omis_type_i8_t : omis_type_object_t {
        omis_type_i8_t(omis_module_t* module)
                : omis_type_object_t(module) {}
    };

    struct omis_type_i16_t : omis_type_object_t {
        omis_type_i16_t(omis_module_t* module)
                : omis_type_object_t(module) {}
    };

    struct omis_type_i32_t : omis_type_object_t {
        omis_type_i32_t(omis_module_t* module)
                : omis_type_object_t(module) {}
    };

    struct omis_type_i64_t : omis_type_object_t {
        omis_type_i64_t(omis_module_t* module)
                : omis_type_object_t(module) {}
    };

    struct omis_type_u8_t : omis_type_object_t {
        omis_type_u8_t(omis_module_t* module)
                : omis_type_object_t(module) {}
    };

    struct omis_type_u16_t : omis_type_object_t {
        omis_type_u16_t(omis_module_t* module)
                : omis_type_object_t(module) {}
    };

    struct omis_type_u32_t : omis_type_object_t {
        omis_type_u32_t(omis_module_t* module)
                : omis_type_object_t(module) {}
    };

    struct omis_type_u64_t : omis_type_object_t {
        omis_type_u64_t(omis_module_t* module)
                : omis_type_object_t(module) {}
    };

    struct omis_type_f32_t : omis_type_object_t {
        omis_type_f32_t(omis_module_t* module)
                : omis_type_object_t(module) {}
    };

    struct omis_type_f64_t : omis_type_object_t {
        omis_type_f64_t(omis_module_t* module)
                : omis_type_object_t(module) {}
    };

    struct omis_type_bool_t : omis_type_object_t {
        omis_type_bool_t(omis_module_t* module)
                : omis_type_object_t(module) {}
    };

    struct omis_type_string_t : omis_type_object_t {
        omis_type_string_t(omis_module_t* module);

        virtual bool main() override;

        omis_value_t* length();
        omis_value_t* toUpper();
        omis_value_t* toLower();
    };

    struct omis_module_core_t : omis_module_t {
        omis_module_core_t(omis_context_t* context);

        virtual bool main() override;

        void define_func_print();
    };
}

#endif //_EOKAS_LLVM_MODULE_CORE_H_
