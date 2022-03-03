
#ifndef _EOKAS_LLVM_TYPES_H_
#define _EOKAS_LLVM_TYPES_H_

#include "./models.h"

_BeginNamespace(eokas)

struct llvm_expr_t;
struct llvm_type_t;
struct llvm_module_t;

struct llvm_typedef_ref_t
{
	static llvm_type_t* define_type(llvm_module_t* module, llvm_type_t* contained_type);
};

struct llvm_typedef_string_t
{
	static llvm_type_t* define_type(llvm_module_t* module);
	static llvm::Value* define_func_intToString(llvm_module_t* module);
	static llvm::Value* define_func_floatToString(llvm_module_t* module);
};

_EndNamespace(eokas)

#endif //_EOKAS_LLVM_TYPES_H_
