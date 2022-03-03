
#include "./types.h"

_BeginNamespace(eokas)

llvm_type_t* llvm_typedef_ref_t::define_type(llvm_module_t* module, llvm_type_t* contained_type)
{
	String name = String::format("Ref<%s>", contained_type->name.cstr());
	llvm_type_t* type = module->new_type(name, nullptr, nullptr);
	type->ref(contained_type);
	return type;
}

llvm_type_t* llvm_typedef_string_t::define_type(llvm_module_t* module)
{
	String name = "String";
	llvm_type_t* type = module->new_type(name, nullptr, nullptr);
	type->add_member("value", module->type_i8_ref, module->new_expr(module->type_i8->defval));
	type->resolve();
	return type;
}

_EndNamespace(eokas)
