
#ifndef _EOKAS_OMIS_BRIDGE_H_
#define _EOKAS_OMIS_BRIDGE_H_

#include "./header.h"

namespace eokas {

    struct omis_bridge_t {
        virtual ~omis_bridge_t() = default;

        virtual omis_handle_t make_module(const String& name) = 0;
        virtual void drop_module(omis_handle_t mod) = 0;
        virtual String dump_module(omis_handle_t mod) = 0;

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
        virtual omis_handle_t type_pointer(omis_handle_t type) = 0;
        virtual omis_handle_t type_func(omis_handle_t ret, const std::vector<omis_handle_t>& args, bool varg) = 0;
        virtual bool is_type_void(omis_handle_t type) = 0;
        virtual bool is_type_i8(omis_handle_t type) = 0;
        virtual bool is_type_i16(omis_handle_t type) = 0;
        virtual bool is_type_i32(omis_handle_t type) = 0;
        virtual bool is_type_i64(omis_handle_t type) = 0;
        virtual bool is_type_f32(omis_handle_t type) = 0;
        virtual bool is_type_f64(omis_handle_t type) = 0;
        virtual bool is_type_bool(omis_handle_t type) = 0;
        virtual bool is_type_bytes(omis_handle_t type) = 0;
        virtual bool is_type_func(omis_handle_t type) = 0;
        virtual bool is_type_array(omis_handle_t type) = 0;
        virtual bool is_type_struct(omis_handle_t type) = 0;
		virtual String get_type_name(omis_handle_t type) = 0;
        virtual omis_handle_t get_type_size(omis_handle_t type) = 0;
        virtual bool can_losslessly_cast(omis_handle_t a, omis_handle_t b) = 0;
		
        virtual omis_handle_t get_func_ret_type(omis_handle_t type_func) = 0;
        virtual uint32_t get_func_arg_count(omis_handle_t type_func) = 0;
        virtual omis_handle_t get_func_arg_type(omis_handle_t type_func, uint32_t index) = 0;
        virtual omis_handle_t get_func_arg_value(omis_handle_t func, uint32_t index) = 0;
        virtual omis_handle_t get_default_value(omis_handle_t type) = 0;

        virtual omis_handle_t value_integer(uint64_t val, uint32_t bits) = 0;
        virtual omis_handle_t value_float(double val) = 0;
        virtual omis_handle_t value_bool(bool val) = 0;
        virtual omis_handle_t value_func(omis_handle_t mod, const String& name, omis_handle_t type) = 0;
        // virtual omis_handle_t value_array(omis_handle_t element_type) = 0;

        virtual omis_handle_t get_value_type(omis_handle_t value) = 0;
        virtual void set_value_name(omis_handle_t value, const String& name) = 0;

        virtual omis_handle_t create_block(omis_handle_t func, const String& name) = 0;
        virtual omis_handle_t get_active_block() = 0;
        virtual void set_active_block(omis_handle_t block) = 0;
        virtual omis_handle_t get_block_tail(omis_handle_t block) = 0;
        virtual bool is_terminator_ins(omis_handle_t ins) = 0;

        virtual omis_handle_t alloc(omis_handle_t type, const String& name = "") = 0;
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
        virtual omis_handle_t ret(omis_handle_t value = nullptr) = 0;
        virtual omis_handle_t bitcast(omis_handle_t value, omis_handle_t type) = 0;

        virtual omis_handle_t get_ptr_val(omis_handle_t ptr) = 0;
        virtual omis_handle_t get_ptr_ref(omis_handle_t ptr) = 0;
		
        virtual bool jit(omis_handle_t module) = 0;
        virtual bool aot(omis_handle_t module) = 0;
    };
}

#endif //_EOKAS_OMIS_BRIDGE_H_
