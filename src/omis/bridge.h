
#ifndef _EOKAS_OMIS_BRIDGE_H_
#define _EOKAS_OMIS_BRIDGE_H_

#include "./header.h"

namespace eokas {
    struct omis_bridge_t {
        virtual omis_handle_t type_void() = 0;
        virtual omis_handle_t type_i8() = 0;
        virtual omis_handle_t type_i16() = 0;
        virtual omis_handle_t type_i32() = 0;
        virtual omis_handle_t type_i64() = 0;
        virtual omis_handle_t type_u8() = 0;
        virtual omis_handle_t type_u16() = 0;
        virtual omis_handle_t type_u32() = 0;
        virtual omis_handle_t type_u64() = 0;
        virtual omis_handle_t type_f32() = 0;
        virtual omis_handle_t type_f64() = 0;
        virtual omis_handle_t type_bool() = 0;
        virtual omis_handle_t type_cstr() = 0;

        virtual omis_handle_t constant_integer(int64_t val) = 0;
        virtual omis_handle_t constant_float(double val) = 0;
        virtual omis_handle_t constant_bool(bool val) = 0;
        virtual omis_handle_t constant_cstr(const char* val) = 0;
        virtual omis_handle_t create_array(omis_handle_t element_type) = 0;
        virtual omis_handle_t create_func(const String& name, omis_handle_t ret_type, const std::vector<omis_handle_t>& args_types) = 0;

        virtual omis_handle_t load(omis_handle_t ptr) = 0;
        virtual void store(omis_handle_t ptr, omis_handle_t val) = 0;
        virtual omis_handle_t gep(omis_handle_t ptr, size_t index, size_t offset) = 0;

        virtual omis_handle_t neg(omis_handle_t a) = 0;
        virtual omis_handle_t add(omis_handle_t a, omis_handle_t b) = 0;
        virtual omis_handle_t sub(omis_handle_t a, omis_handle_t b) = 0;
        virtual omis_handle_t mul(omis_handle_t a, omis_handle_t b) = 0;
        virtual omis_handle_t div(omis_handle_t a, omis_handle_t b) = 0;
        virtual omis_handle_t mod(omis_handle_t a, omis_handle_t b) = 0;
        virtual omis_handle_t cmp(omis_handle_t a, omis_handle_t b) = 0;
        virtual omis_handle_t l_not(omis_handle_t a) = 0;
        virtual omis_handle_t l_and(omis_handle_t a, omis_handle_t b) = 0;
        virtual omis_handle_t l_or(omis_handle_t a, omis_handle_t b) = 0;
        virtual omis_handle_t b_flip(omis_handle_t a) = 0;
        virtual omis_handle_t b_and(omis_handle_t a, omis_handle_t b) = 0;
        virtual omis_handle_t b_or(omis_handle_t a, omis_handle_t b) = 0;
        virtual omis_handle_t b_xor(omis_handle_t a, omis_handle_t b) = 0;
        virtual omis_handle_t b_shl(omis_handle_t a, omis_handle_t b) = 0;
        virtual omis_handle_t b_shr(omis_handle_t a, omis_handle_t b) = 0;

        virtual void jump(omis_handle_t pos) = 0;
        virtual omis_handle_t call(omis_handle_t func, const std::vector<omis_handle_t>& args) = 0;
        virtual void ret(omis_handle_t value) = 0;

        virtual omis_handle_t make(omis_handle_t type, omis_handle_t count) = 0;
        virtual void drop(omis_handle_t ptr) = 0;

    };
}

#endif //_EOKAS_OMIS_BRIDGE_H_
