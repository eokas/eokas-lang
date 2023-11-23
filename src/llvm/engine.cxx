#include "engine.h"
#include "coder.h"
#include "scope.h"
#include "../omis/bridge.h"

#include <llvm/ADT/APFloat.h>
#include <llvm/ADT/STLExtras.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Mangler.h>
#include <llvm/IR/LegacyPassManager.h>

#include <llvm/Support/FileSystem.h>
#include <llvm/Support/Host.h>
#include <llvm/Support/TargetRegistry.h>
#include <llvm/Support/TargetSelect.h>

#include <llvm/Target/TargetMachine.h>
#include <llvm/Target/TargetOptions.h>

#include <llvm/ExecutionEngine/MCJIT.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/GenericValue.h>

namespace eokas
{
#define _Ty(handle) ((llvm::Type*)handle)
#define _Val(handle) ((llvm::Value*)handle)
#define _Func(handle) ((llvm::Function*)handle)
#define _Block(handle) ((llvm::BasicBlock*)handle)

    struct llvm_bridge_t :public omis_bridge_t {
        llvm::LLVMContext& context;
        llvm::IRBuilder<> IR;

        llvm::Type* ty_void;
        llvm::Type* ty_i8;
        llvm::Type* ty_i16;
        llvm::Type* ty_i32;
        llvm::Type* ty_i64;
        llvm::Type* ty_u8;
        llvm::Type* ty_u16;
        llvm::Type* ty_u32;
        llvm::Type* ty_u64;
        llvm::Type* ty_f32;
        llvm::Type* ty_f64;
        llvm::Type* ty_bool;
        llvm::Type* ty_bytes;
        llvm::Type* ty_void_ptr;

        llvm_bridge_t(llvm::LLVMContext& context)
                : omis_bridge_t(), context(context), IR(context) {
            ty_void = llvm::Type::getVoidTy(context);
            ty_i8 = llvm::Type::getInt8Ty(context);
            ty_i16 = llvm::Type::getInt16Ty(context);
            ty_i32 = llvm::Type::getInt32Ty(context);
            ty_i64 = llvm::Type::getInt64Ty(context);
            ty_u8 = llvm::Type::getInt8Ty(context);
            ty_u16 = llvm::Type::getInt16Ty(context);
            ty_u32 = llvm::Type::getInt32Ty(context);
            ty_u64 = llvm::Type::getInt64Ty(context);
            ty_f32 = llvm::Type::getFloatTy(context);
            ty_f64 = llvm::Type::getDoubleTy(context);
            ty_bool = llvm::Type::getInt1Ty(context);
            ty_bytes = llvm::Type::getInt8PtrTy(context);
            ty_void_ptr = ty_void->getPointerTo();
        }

        virtual bool can_losslessly_cast(omis_handle_t a, omis_handle_t b) override {
            auto* aT = (llvm::Type*)a;
            auto* bT = (llvm::Type*)b;
            return aT->canLosslesslyBitCastTo(bT);
        }

        virtual omis_handle_t type_void() override {
            return ty_void;
        }

        virtual omis_handle_t type_i8() override {
            return ty_i8;
        }

        virtual omis_handle_t type_i16() override {
            return ty_i16;
        }

        virtual omis_handle_t type_i32() override {
            return ty_i32;
        }

        virtual omis_handle_t type_i64() override {
            return ty_i64;
        }

        virtual omis_handle_t type_u8() override {
            return ty_u8;
        }

        virtual omis_handle_t type_u16() override {
            return ty_u16;
        }

        virtual omis_handle_t type_u32() override {
            return ty_u32;
        }

        virtual omis_handle_t type_u64() override {
            return ty_u64;
        }

        virtual omis_handle_t type_f32() override {
            return ty_f32;
        }

        virtual omis_handle_t type_f64() override {
            return ty_f64;
        }

        virtual omis_handle_t type_bool() override {
            return llvm::Type::getInt1Ty(context);
        }

        virtual omis_handle_t type_bytes() override {
            return ty_bytes;
        }

        virtual omis_handle_t constant_integer(uint64_t val, uint32_t bits) override {
            auto* type = ty_i64;
            if(bits == 32) type = ty_i32;
            return llvm::Constant::getIntegerValue(type, llvm::APInt(bits, val, true));
        }

        virtual omis_handle_t constant_float(double val) override {
            return llvm::ConstantFP::get(context, llvm::APFloat(val));
        }

        virtual omis_handle_t constant_bool(bool val) override {
            return llvm::Constant::getIntegerValue(ty_bool, llvm::APInt(1, val ? 1 : 0, true));
        }

        // virtual omis_handle_t create_array(omis_handle_t element_type) = 0;
        // virtual omis_handle_t create_func(const String& name, omis_handle_t ret_type, const std::vector<omis_handle_t>& args_types) = 0;

        virtual omis_handle_t create_block(omis_handle_t func, const String& name) override {
            return llvm::BasicBlock::Create(context, name.cstr(), _Func(func));
        }

        virtual void activate_block(omis_handle_t block) override {
            IR.SetInsertPoint(_Block(block));
        }

        virtual omis_handle_t alloc(omis_handle_t type) override {
            return IR.CreateAlloca(_Ty(type));
        }

        virtual omis_handle_t load(omis_handle_t ptr) override {
            return IR.CreateLoad(_Val(ptr));
        }

        virtual omis_handle_t store(omis_handle_t ptr, omis_handle_t val) override {
            return IR.CreateStore(_Val(val), _Val(ptr));
        }

        virtual omis_handle_t gep(omis_handle_t type, omis_handle_t ptr, omis_handle_t index) override {
            return IR.CreateGEP(_Ty(type), _Val(ptr), _Val(index));
        }

        virtual omis_handle_t neg(omis_handle_t a) = 0;
        virtual omis_handle_t add(omis_handle_t a, omis_handle_t b) = 0;
        virtual omis_handle_t sub(omis_handle_t a, omis_handle_t b) = 0;
        virtual omis_handle_t mul(omis_handle_t a, omis_handle_t b) = 0;
        virtual omis_handle_t div(omis_handle_t a, omis_handle_t b) = 0;
        virtual omis_handle_t mod(omis_handle_t a, omis_handle_t b) = 0;
        virtual omis_handle_t cmp(omis_handle_t a, omis_handle_t b) = 0;
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
        virtual omis_handle_t phi(omis_handle_t type, size_t incommings) = 0;
        virtual void phi_set_incomming(omis_handle_t phi, omis_handle_t value, omis_handle_t block) = 0;
        virtual omis_handle_t call(omis_handle_t func, const std::vector<omis_handle_t>& args) = 0;
        virtual omis_handle_t ret(omis_handle_t value) = 0;

        virtual omis_handle_t make(omis_handle_t type, omis_handle_t count) = 0;
        virtual void drop(omis_handle_t ptr) = 0;
    };


	bool llvm_jit(ast_node_module_t* m)
	{
		llvm::InitializeNativeTarget();
		llvm::InitializeNativeTargetAsmPrinter();
		llvm::InitializeNativeTargetAsmParser();
		
		llvm::LLVMContext context;
		
		llvm_module_t* module = llvm_encode(context, m);
		if(module == nullptr)
			return false;
		
		module->module.print(llvm::errs(), nullptr);
		
		auto ee = llvm::EngineBuilder(std::unique_ptr<llvm::Module>(&module->module)).setEngineKind(llvm::EngineKind::JIT).create();
		
		ee->finalizeObject();
		
		llvm::Function* func = module->module.getFunction("main");
		if(func == nullptr)
			return false;
		
		printf("---------------- JIT RUN ----------------\n");
		std::vector<llvm::GenericValue> args;
		auto retval = ee->runFunction(func, args);
		printf("\nRET: %s \n", retval.IntVal.toString(10, true).c_str());
		printf("---------------- JIT END ----------------\n");
		
		_DeletePointer(module);
		
		return true;
	}
	
	bool llvm_aot(ast_node_module_t* m)
	{
		llvm::InitializeAllTargetInfos();
		llvm::InitializeAllTargets();
		llvm::InitializeAllTargetMCs();
		llvm::InitializeAllAsmParsers();
		llvm::InitializeAllAsmPrinters();
		
		llvm::LLVMContext context;
		
		llvm_module_t* module = llvm_encode(context, m);
		if(module == nullptr)
			return false;
		
		// DUMP CODE
		module->module.print(llvm::errs(), nullptr);
		
		auto targetTriple = llvm::sys::getDefaultTargetTriple();
		std::string error;
		auto target = llvm::TargetRegistry::lookupTarget(targetTriple, error);
		
		auto CPU = "generic";
		auto features = "";
		llvm::TargetOptions opt;
		auto RM = llvm::Optional<llvm::Reloc::Model>();
		auto targetMachine = target->createTargetMachine(targetTriple, CPU, features, opt, RM);
		
		module->module.setDataLayout(targetMachine->createDataLayout());
		module->module.setTargetTriple(targetTriple);
		
		auto filename = "output.o";
		std::error_code EC;
		llvm::raw_fd_ostream dest(filename, EC, llvm::sys::fs::OF_None);
		if(EC)
		{
			llvm::errs() << "Could not open file: " << EC.message();
			return false;
		}
		
		llvm::legacy::PassManager pass;
		auto fileType = llvm::CGFT_ObjectFile;
		if(targetMachine->addPassesToEmitFile(pass, dest, nullptr, fileType))
		{
			llvm::errs() << "TargetMachine can't emit a file of this type";
			return false;
		}
		
		pass.run(module->module);
		dest.flush();
		
		_DeletePointer(module);
		
		return true;
	}
}
