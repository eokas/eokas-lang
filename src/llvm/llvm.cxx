#include "llvm.h"
#include "../llvm-old/coder.h"
#include "../llvm-old/scope.h"
#include "../omis/bridge.h"
#include "../omis/model.h"

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
        llvm::Module module;
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

        llvm_bridge_t(const String& name, llvm::LLVMContext& context)
                : omis_bridge_t(), context(context), module(name.cstr(), context), IR(context) {
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

        virtual omis_handle_t type_func(omis_handle_t ret, const std::vector<omis_handle_t>& args, bool varg) override {
            llvm::Type* ret_type = _Ty(ret);
            std::vector<llvm::Type*> args_type;
            for(auto& arg : args) {
                args_type.push_back(_Ty(arg));
            }
            llvm::FunctionType* funcType = llvm::FunctionType::get(ret_type, args_type, varg);
            return funcType;
        }

        virtual bool can_losslessly_cast(omis_handle_t a, omis_handle_t b) override {
            auto* aT = (llvm::Type*)a;
            auto* bT = (llvm::Type*)b;
            return aT->canLosslesslyBitCastTo(bT);
        }

        virtual omis_handle_t value_integer(uint64_t val, uint32_t bits) override {
            auto* type = ty_i64;
            if(bits == 32) type = ty_i32;
            return llvm::Constant::getIntegerValue(type, llvm::APInt(bits, val, true));
        }

        virtual omis_handle_t value_float(double val) override {
            return llvm::ConstantFP::get(context, llvm::APFloat(val));
        }

        virtual omis_handle_t value_bool(bool val) override {
            return llvm::Constant::getIntegerValue(ty_bool, llvm::APInt(1, val ? 1 : 0, true));
        }

        virtual omis_handle_t value_func(const String& name, omis_handle_t type) override {
            llvm::FunctionType* funcType = (llvm::FunctionType*)type;
            llvm::Function* funcPtr = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, name.cstr(), module);

            llvm::AttributeList attrs;
            funcPtr->setAttributes(attrs);
            funcPtr->setCallingConv(llvm::CallingConv::C);

            return funcPtr;
        }

        // virtual omis_handle_t create_array(omis_handle_t element_type) = 0;

        virtual omis_handle_t get_value_type(omis_handle_t value) override {
            return _Val(value)->getType();
        }

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

        virtual omis_handle_t neg(omis_handle_t a) override {
            auto rhs = _Val(a);
            auto rtype = rhs->getType();

            if (rtype->isIntegerTy())
                return IR.CreateNeg(rhs);

            if (rtype->isFloatingPointTy())
                return IR.CreateFNeg(rhs);

            printf("Type of RHS is invalid.\n");
            return nullptr;
        }

        enum class ArithOp {ADD, SUB, MUL, DIV, MOD};
        omis_handle_t arith(ArithOp op, omis_handle_t a, omis_handle_t b) {
            using ins_type_t = std::function<llvm::Value *(llvm::IRBuilder<> &IR, llvm::Value *LHS, llvm::Value *RHS)>;
            using func_type_t = llvm::Value *(llvm::IRBuilder<>::*)(llvm::Value *LHS, llvm::Value *RHS);

            static std::map<ArithOp, ins_type_t> ins_i = {
                {ArithOp::ADD, (func_type_t) &llvm::IRBuilder<>::CreateAdd},
                {ArithOp::SUB, (func_type_t) &llvm::IRBuilder<>::CreateSub},
                {ArithOp::MUL, (func_type_t) &llvm::IRBuilder<>::CreateMul},
                {ArithOp::DIV, (func_type_t) &llvm::IRBuilder<>::CreateSDiv},
                {ArithOp::MOD, (func_type_t) &llvm::IRBuilder<>::CreateSRem},
            };

            static std::map<ArithOp, ins_type_t> ins_f = {
                {ArithOp::ADD, (func_type_t) &llvm::IRBuilder<>::CreateFAdd},
                {ArithOp::SUB, (func_type_t) &llvm::IRBuilder<>::CreateFSub},
                {ArithOp::MUL, (func_type_t) &llvm::IRBuilder<>::CreateFMul},
                {ArithOp::DIV, (func_type_t) &llvm::IRBuilder<>::CreateFDiv},
                {ArithOp::MOD, (func_type_t) &llvm::IRBuilder<>::CreateFRem},
            };

            auto lhs = _Val(a);
            auto rhs = _Val(b);
            auto ltype = lhs->getType();
            auto rtype = rhs->getType();

            if (ltype->isIntegerTy() && rtype->isIntegerTy())
                return ins_i[op](IR, lhs, rhs);

            if (ltype->isFloatingPointTy() && rtype->isFloatingPointTy())
                return ins_f[op](IR, lhs, rhs);

            if (ltype->isIntegerTy() && rtype->isFloatingPointTy()) {
                lhs = IR.CreateSIToFP(lhs, llvm::Type::getDoubleTy(context));
                return ins_f[op](IR, lhs, rhs);
            }

            if (ltype->isFloatingPointTy() && rtype->isIntegerTy()) {
                rhs = IR.CreateSIToFP(rhs, llvm::Type::getDoubleTy(context));
                return ins_f[op](IR, lhs, rhs);
            }

            printf("Type of LHS or RHS is invalid.\n");
            return nullptr;
        }

        virtual omis_handle_t add(omis_handle_t a, omis_handle_t b) override {
            return this->arith(ArithOp::ADD, a, b);
        }

        virtual omis_handle_t sub(omis_handle_t a, omis_handle_t b) override {
            return this->arith(ArithOp::SUB, a, b);
        }

        virtual omis_handle_t mul(omis_handle_t a, omis_handle_t b) override {
            return this->arith(ArithOp::MUL, a, b);
        }

        virtual omis_handle_t div(omis_handle_t a, omis_handle_t b) override {
            return this->arith(ArithOp::DIV, a, b);
        }

        virtual omis_handle_t mod(omis_handle_t a, omis_handle_t b) override {
            return this->arith(ArithOp::MOD, a, b);
        }

        enum class CmpOp {EQ, NE, LE, GE, LT, GT};
        omis_handle_t cmp(CmpOp op, omis_handle_t a, omis_handle_t b) {
            static std::map<CmpOp, llvm::CmpInst::Predicate> op_i = {
                {CmpOp::EQ, llvm::CmpInst::ICMP_EQ},
                {CmpOp::NE, llvm::CmpInst::ICMP_NE},
                {CmpOp::LE, llvm::CmpInst::ICMP_SLE},
                {CmpOp::GE, llvm::CmpInst::ICMP_SGE},
                {CmpOp::LT, llvm::CmpInst::ICMP_SLT},
                {CmpOp::GT, llvm::CmpInst::ICMP_SGT},
            };

            static std::map<CmpOp, llvm::CmpInst::Predicate> op_f = {
                {CmpOp::EQ, llvm::CmpInst::FCMP_OEQ},
                {CmpOp::NE, llvm::CmpInst::FCMP_ONE},
                {CmpOp::LE, llvm::CmpInst::FCMP_OLE},
                {CmpOp::GE, llvm::CmpInst::FCMP_OGE},
                {CmpOp::LT, llvm::CmpInst::FCMP_OLT},
                {CmpOp::GT, llvm::CmpInst::FCMP_OGT},
            };

            auto lhs = _Val(a);
            auto rhs = _Val(b);
            auto ltype = lhs->getType();
            auto rtype = rhs->getType();

            if (ltype->isIntegerTy() && rtype->isIntegerTy())
                return IR.CreateCmp(op_i[op], lhs, rhs, "", nullptr);

            if (ltype->isPointerTy() && rtype->isPointerTy()) {
                lhs = IR.CreatePtrToInt(lhs, llvm::Type::getInt64Ty(context));
                rhs = IR.CreatePtrToInt(rhs, llvm::Type::getInt64Ty(context));
                return IR.CreateCmp(op_i[op], lhs, rhs, "", nullptr);
            }

            if (ltype->isFloatingPointTy() && rtype->isFloatingPointTy())
                return IR.CreateCmp(op_f[op], lhs, rhs, "", nullptr);

            if (ltype->isIntegerTy() && rtype->isFloatingPointTy()) {
                lhs = IR.CreateSIToFP(lhs, llvm::Type::getDoubleTy(context));
                return IR.CreateCmp(op_f[op], lhs, rhs, "", nullptr);
            }

            if (ltype->isFloatingPointTy() && rtype->isIntegerTy()) {
                rhs = IR.CreateSIToFP(rhs, llvm::Type::getDoubleTy(context));
                return IR.CreateCmp(op_f[op], lhs, rhs, "", nullptr);
            }

            printf("Type of LHS or RHS is invalid.\n");
            return nullptr;
        }

        virtual omis_handle_t eq(omis_handle_t a, omis_handle_t b) override {
            return this->cmp(CmpOp::EQ, a, b);
        }

        virtual omis_handle_t ne(omis_handle_t a, omis_handle_t b) override {
            return this->cmp(CmpOp::NE, a, b);
        }

        virtual omis_handle_t gt(omis_handle_t a, omis_handle_t b) override {
            return this->cmp(CmpOp::GT, a, b);
        }

        virtual omis_handle_t ge(omis_handle_t a, omis_handle_t b) override {
            return this->cmp(CmpOp::GE, a, b);
        }

        virtual omis_handle_t lt(omis_handle_t a, omis_handle_t b) override {
            return this->cmp(CmpOp::LT, a, b);
        }

        virtual omis_handle_t le(omis_handle_t a, omis_handle_t b) override {
            return this->cmp(CmpOp::LE, a, b);
        }

        virtual omis_handle_t l_not(omis_handle_t a) override {
            auto rhs = _Val(a);
            auto rtype = rhs->getType();

            if (rtype->isIntegerTy() && rtype->getIntegerBitWidth() == 1)
                return IR.CreateNot(rhs);

            printf("Type of RHS is invalid.\n");
            return nullptr;
        }

        virtual omis_handle_t l_and(omis_handle_t a, omis_handle_t b) override {
            auto lhs = _Val(a);
            auto rhs = _Val(b);
            auto ltype = lhs->getType();
            auto rtype = rhs->getType();

            if (ltype->isIntegerTy(1) && rtype->isIntegerTy(1)) {
                return IR.CreateAnd(lhs, rhs);
            }

            printf("LHS or RHS is not bool value. \n");
            return nullptr;
        }

        virtual omis_handle_t l_or(omis_handle_t a, omis_handle_t b) override {
            auto lhs = _Val(a);
            auto rhs = _Val(b);
            auto ltype = lhs->getType();
            auto rtype = rhs->getType();

            if (ltype->isIntegerTy(1) && rtype->isIntegerTy(1)) {
                return IR.CreateOr(lhs, rhs);
            }

            printf("LHS or RHS is not bool value. \n");
            return nullptr;
        }

        virtual omis_handle_t b_flip(omis_handle_t a) override {
            auto rhs _Val(a);
            auto rtype = rhs->getType();

            if (rtype->isIntegerTy()) {
                auto bits = rtype->getIntegerBitWidth();
                auto mask = llvm::ConstantInt::get(rtype, llvm::APInt(bits, 0xFFFFFFFF));
                return IR.CreateXor(rhs, mask);
            }

            printf("Type of RHS is invalid.\n");
            return nullptr;
        }

        virtual omis_handle_t b_and(omis_handle_t a, omis_handle_t b) override {
            auto lhs = _Val(a);
            auto rhs = _Val(b);
            auto ltype = lhs->getType();
            auto rtype = rhs->getType();

            if (ltype->isIntegerTy() && rtype->isIntegerTy()) {
                return IR.CreateAnd(lhs, rhs);
            }

            printf("Type of LHS or RHS is invalid.\n");
            return nullptr;
        }

        virtual omis_handle_t b_or(omis_handle_t a, omis_handle_t b) override {
            auto lhs = _Val(a);
            auto rhs = _Val(b);
            auto ltype = lhs->getType();
            auto rtype = rhs->getType();

            if (ltype->isIntegerTy() && rtype->isIntegerTy()) {
                return IR.CreateOr(lhs, rhs);
            }

            printf("Type of LHS or RHS is invalid.\n");
            return nullptr;
        }

        virtual omis_handle_t b_xor(omis_handle_t a, omis_handle_t b) override {
            auto lhs = _Val(a);
            auto rhs = _Val(b);
            auto ltype = lhs->getType();
            auto rtype = rhs->getType();

            if (ltype->isIntegerTy() && rtype->isIntegerTy()) {
                return IR.CreateXor(lhs, rhs);
            }

            printf("Type of LHS or RHS is invalid.\n");
            return nullptr;
        }

        virtual omis_handle_t b_shl(omis_handle_t a, omis_handle_t b) override {
            auto lhs = _Val(a);
            auto rhs = _Val(b);
            auto ltype = lhs->getType();
            auto rtype = rhs->getType();

            if (ltype->isIntegerTy() && rtype->isIntegerTy()) {
                return IR.CreateShl(lhs, rhs);
            }

            printf("Type of LHS or RHS is invalid.\n");
            return nullptr;
        }

        virtual omis_handle_t b_shr(omis_handle_t a, omis_handle_t b) override {
            auto lhs = _Val(a);
            auto rhs = _Val(b);
            auto ltype = lhs->getType();
            auto rtype = rhs->getType();

            if (ltype->isIntegerTy() && rtype->isIntegerTy()) {
                // CreateLShr: 逻辑右移：在左边补 0
                // CreateShr: 算术右移：在左边补 符号位
                // 我们采用逻辑右移
                return IR.CreateLShr(lhs, rhs);
            }

            printf("Type of LHS or RHS is invalid.\n");
            return nullptr;
        }

        virtual omis_handle_t jump(omis_handle_t block) override {
            return IR.CreateBr(_Block(block));
        }

        virtual omis_handle_t jump_cond(omis_handle_t cond, omis_handle_t branch_true, omis_handle_t branch_false) override {
            return IR.CreateCondBr(_Val(cond), _Block(branch_true), _Block(branch_false));
        }

        virtual omis_handle_t phi(omis_handle_t type, const std::map<omis_handle_t, omis_handle_t>& incomings) override {
            auto phi = IR.CreatePHI(_Ty(type), incomings.size());
            for(auto& pair : incomings) {
                phi->addIncoming(_Val(pair.first), _Block(pair.second));
            }
        }

        virtual omis_handle_t call(omis_handle_t func, const std::vector<omis_handle_t>& args) override {
            std::vector<llvm::Value*> args_values;
            for(auto& arg : args) {
                args_values.push_back(_Val(arg));
            }
            return IR.CreateCall(_Func(func), args_values);
        }

        virtual omis_handle_t ret(omis_handle_t value) override {
            return IR.CreateRet(_Val(value));
        }

        // virtual omis_handle_t make(omis_handle_t type, omis_handle_t count) = 0;
        // virtual void drop(omis_handle_t ptr) = 0;
    };

	bool llvm_jit(ast_node_module_t* m)
	{
		llvm::InitializeNativeTarget();
		llvm::InitializeNativeTargetAsmPrinter();
		llvm::InitializeNativeTargetAsmParser();
		
		llvm::LLVMContext context;
        llvm_bridge_t bridge(m->name, context);
        omis_module_t module(m->name, &bridge);

        module

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
