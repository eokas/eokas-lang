
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
	type->add_member("value", module->type_i8_ref, module->type_i8->defval);
	type->resolve();
	return type;
}

llvm::Value* llvm_typedef_string_t::define_func_intToString(llvm_module_t* module)
{
	String name = "intToString";
	llvm::Type* ret = module->type_string_ref->handle;
	std::vector<llvm::Type*> args = {module->type_i32->handle};
	bool varg = false;
	
	auto func = llvm_model_t::define_func(module->module, name, ret, args, varg, [&](llvm::LLVMContext& ctx, llvm::Module& mod, llvm::Function* func, llvm::IRBuilder<>& builder)->void
	{
		llvm::BasicBlock* entry = llvm::BasicBlock::Create(ctx, "entry", func);
		builder.SetInsertPoint(entry);
		
		llvm::Value* arg0 = func->getArg(0);
		llvm::Value* str = module->value_to_string(func, builder, arg0);
		
		builder.CreateRet(str);
	});
	
	return func;
}

llvm::Value* llvm_typedef_string_t::define_func_floatToString(llvm_module_t* module)
{
	String name = "floatToString";
	llvm::Type* ret = module->type_string_ref->handle;
	std::vector<llvm::Type*> args = {module->type_f32->handle};
	bool varg = false;
	
	auto func = llvm_model_t::define_func(module->module, name, ret, args, varg, [&](llvm::LLVMContext& ctx, llvm::Module& mod, llvm::Function* func, llvm::IRBuilder<>& builder)->void
	{
		llvm::BasicBlock* entry = llvm::BasicBlock::Create(ctx, "entry", func);
		builder.SetInsertPoint(entry);
		
		llvm::Value* arg0 = func->getArg(0);
		llvm::Value* str = module->value_to_string(func, builder, arg0);
		
		builder.CreateRet(str);
	});
	
	return func;
}

_EndNamespace(eokas)
