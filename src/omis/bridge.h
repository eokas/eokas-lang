
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
        virtual omis_handle_t type_bytes() = 0;
        virtual omis_handle_t type_func(omis_handle_t ret, const std::vector<omis_handle_t>& args) = 0;
        virtual bool can_losslessly_cast(omis_handle_t a, omis_handle_t b) = 0;

        virtual omis_handle_t value_integer(uint64_t val, uint32_t bits) = 0;
        virtual omis_handle_t value_float(double val) = 0;
        virtual omis_handle_t value_bool(bool val) = 0;
        virtual omis_handle_t value_func(const String& name, omis_handle_t type) = 0;
        // virtual omis_handle_t value_array(omis_handle_t element_type) = 0;

        virtual omis_handle_t create_block(omis_handle_t func, const String& name) = 0;
        virtual void activate_block(omis_handle_t block) = 0;

        virtual omis_handle_t alloc(omis_handle_t type) = 0;
        virtual omis_handle_t load(omis_handle_t ptr) = 0;
        virtual omis_handle_t store(omis_handle_t ptr, omis_handle_t val) = 0;
        virtual omis_handle_t gep(omis_handle_t type, omis_handle_t ptr, omis_handle_t index) = 0;

        virtual omis_handle_t neg(omis_handle_t a) = 0;
        virtual omis_handle_t add(omis_handle_t a, omis_handle_t b) = 0;
        virtual omis_handle_t sub(omis_handle_t a, omis_handle_t b) = 0;
        virtual omis_handle_t mul(omis_handle_t a, omis_handle_t b) = 0;
        virtual omis_handle_t div(omis_handle_t a, omis_handle_t b) = 0;
        virtual omis_handle_t mod(omis_handle_t a, omis_handle_t b) = 0;

        virtual omis_handle_t eq(omis_handle_t a, omis_handle_t b) = 0;
        virtual omis_handle_t ne(omis_handle_t a, omis_handle_t b) = 0;
        virtual omis_handle_t gt(omis_handle_t a, omis_handle_t b) = 0;
        virtual omis_handle_t ge(omis_handle_t a, omis_handle_t b) = 0;
        virtual omis_handle_t lt(omis_handle_t a, omis_handle_t b) = 0;
        virtual omis_handle_t le(omis_handle_t a, omis_handle_t b) = 0;

        virtual omis_handle_t l_not(omis_handle_t a) = 0;
        virtual omis_handle_t l_and(omis_handle_t a, omis_handle_t b) = 0;
        virtual omis_handle_t l_or(omis_handle_t a, omis_handle_t b) = 0;

        virtual omis_handle_t b_flip(omis_handle_t a) = 0;
        virtual omis_handle_t b_and(omis_handle_t a, omis_handle_t b) = 0;
        virtual omis_handle_t b_or(omis_handle_t a, omis_handle_t b) = 0;
        virtual omis_handle_t b_xor(omis_handle_t a, omis_handle_t b) = 0;
        virtual omis_handle_t b_shl(omis_handle_t a, omis_handle_t b) = 0;
        virtual omis_handle_t b_shr(omis_handle_t a, omis_handle_t b) = 0;

        virtual omis_handle_t jump(omis_handle_t pos) = 0;
        virtual omis_handle_t jump_cond(omis_handle_t cond, omis_handle_t branch_true, omis_handle_t branch_false) = 0;
        virtual omis_handle_t phi(omis_handle_t type, const std::map<omis_handle_t, omis_handle_t>& incomings) = 0;
        virtual omis_handle_t call(omis_handle_t func, const std::vector<omis_handle_t>& args) = 0;
        virtual omis_handle_t ret(omis_handle_t value) = 0;

        virtual omis_handle_t make(omis_handle_t type, omis_handle_t count) = 0;
        virtual void drop(omis_handle_t ptr) = 0;

    };
}

#endif //_EOKAS_OMIS_BRIDGE_H_
