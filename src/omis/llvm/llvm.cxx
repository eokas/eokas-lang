#include "llvm.h"

#include "../bridge.h"
#include "../model.h"

#include <sstream>

#include <llvm/ADT/APInt.h>
#include <llvm/ADT/APSInt.h>
#include <llvm/ADT/APFloat.h>
#include <llvm/ADT/STLExtras.h>

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Verifier.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Mangler.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/BasicBlock.h>

#include <llvm/Target/TargetMachine.h>
#include <llvm/Target/TargetOptions.h>

#include <llvm/Support/FileSystem.h>
#include <llvm/Support/Host.h>
#include <llvm/Support/TargetRegistry.h>
#include <llvm/Support/TargetSelect.h>

#include <llvm/ExecutionEngine/MCJIT.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/GenericValue.h>

namespace eokas
{
#define _Mod(handle) ((llvm::Module*)handle)
#define _Ty(handle) ((llvm::Type*)handle)
#define _Val(handle) ((llvm::Value*)handle)
#define _Func(handle) ((llvm::Function*)handle)
#define _Block(handle) ((llvm::BasicBlock*)handle)
#define _Ins(handle) ((llvm::Instruction*)handle)

    struct llvm_bridge_t :public omis_bridge_t {
        llvm::LLVMContext context;
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

        llvm_bridge_t()
            : omis_bridge_t(), context(), IR(context) {
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

        virtual omis_handle_t make_module(const String& name) override {
            llvm::Module* mod = new llvm::Module(name.cstr(), context);
            return mod;
        }

        virtual void drop_module(omis_handle_t mod) override {
            llvm::Module* module = _Mod(mod);
            _DeletePointer(module);
        }

        virtual String dump_module(omis_handle_t mod) override {
            auto* module = _Mod(mod);

            // module->print(llvm::outs(), nullptr);

            std::string str;
            llvm::raw_string_ostream stream(str);
            module->print(stream, nullptr);
            String ret = String{str.c_str(), str.length()}.replace("%", "%%");
            return ret;
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

        virtual omis_handle_t type_pointer(omis_handle_t type) override {
            return _Ty(type)->getPointerTo();
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

        virtual bool is_type_void(omis_handle_t type) override {
            return _Ty(type)->isVoidTy();
        }

        virtual bool is_type_i8(omis_handle_t type) override {
            return _Ty(type)->isIntegerTy(8);
        }

        virtual bool is_type_i16(omis_handle_t type) override {
            return _Ty(type)->isIntegerTy(16);
        }

        virtual bool is_type_i32(omis_handle_t type) override {
            return _Ty(type)->isIntegerTy(32);
        }

        virtual bool is_type_i64(omis_handle_t type) override {
            return _Ty(type)->isIntegerTy(64);
        }

        virtual bool is_type_f32(omis_handle_t type) override {
            return _Ty(type)->isFloatTy();
        }

        virtual bool is_type_f64(omis_handle_t type) override {
            return _Ty(type)->isDoubleTy();
        }

        virtual bool is_type_bool(omis_handle_t type) override {
            return _Ty(type)->isIntegerTy(1);
        }

        virtual bool is_type_bytes(omis_handle_t type) override {
            auto ty = _Ty(type);
            return ty->isPointerTy() && ty->getPointerElementType()->isIntegerTy(8);
        }

        virtual bool is_type_func(omis_handle_t type) override {
            return _Ty(type)->isFunctionTy();
        }

        virtual bool is_type_array(omis_handle_t type) override {
            return _Ty(type)->isArrayTy();
        }

        virtual bool is_type_struct(omis_handle_t type) override {
            return _Ty(type)->isStructTy();
        }
		
		virtual String get_type_name(omis_handle_t type) override {
			auto ty = _Ty(type);
			if(ty->isVoidTy()) return "void";
			if(ty->isIntegerTy(1)) return "bool";
			if(ty->isIntegerTy(8)) return "i8";
			if(ty->isIntegerTy(16)) return "i16";
			if(ty->isIntegerTy(32)) return "i32";
			if(ty->isIntegerTy(64)) return "i64";
			if(ty->isFloatTy()) return "f32";
			if(ty->isDoubleTy()) return "f64";
			if(ty->isPointerTy()) {
				auto eleTy = ty->getPointerElementType();
				return String::format("Pointer<%s>", this->get_type_name(eleTy).cstr());
			}
			if(ty->isArrayTy()) {
				auto eleTy = ty->getArrayElementType();
				return String::format("Array<%s>", this->get_type_name(eleTy).cstr());
			}
			if(ty->isFunctionTy()) {
				String str = "func(";
				uint32_t count = this->get_func_arg_count(ty);
				for(uint32_t i = 0; i < count; i++) {
					auto argTy = this->get_func_arg_type(ty, i);
					str += this->get_type_name(argTy);
					if(i == count-1) {
						str += ", ";
					}
				}
				str += ") : ";
				str += this->get_type_name(this->get_func_ret_type(ty));
				return str;
			}
			if(ty->isStructTy()) {
				return ty->getStructName().data();
			}
			
			return "Unknown";
		}

        virtual omis_handle_t get_type_size(omis_handle_t type) override {
            llvm::Constant* size = llvm::ConstantExpr::getSizeOf(_Ty(type));
            return size;
        }

        virtual bool can_losslessly_cast(omis_handle_t a, omis_handle_t b) override {
            auto* aT = (llvm::Type*)a;
            auto* bT = (llvm::Type*)b;
            return aT->canLosslesslyBitCastTo(bT);
        }

        virtual omis_handle_t get_func_ret_type(omis_handle_t type_func) override {
			auto ty = _Ty(type_func);
			if (ty->isPointerTy() && ty->getPointerElementType()->isFunctionTy()) {
				ty = ty->getPointerElementType();
			}
			auto type = (llvm::FunctionType *) ty;
			return type->getReturnType();
		}

        virtual uint32_t get_func_arg_count(omis_handle_t type_func) override {
			auto ty = _Ty(type_func);
			if (ty->isPointerTy() && ty->getPointerElementType()->isFunctionTy()) {
				ty = ty->getPointerElementType();
			}
			auto type = (llvm::FunctionType *) ty;
            return type->getNumParams();
        }

        virtual omis_handle_t get_func_arg_type(omis_handle_t type_func, uint32_t index) override {
			auto ty = _Ty(type_func);
			if (ty->isPointerTy() && ty->getPointerElementType()->isFunctionTy()) {
				ty = ty->getPointerElementType();
			}
			auto type = (llvm::FunctionType *) ty;
            return type->getParamType(index);
        }

        virtual omis_handle_t get_func_arg_value(omis_handle_t func, uint32_t index) override {
            return _Func(func)->getArg(index);
        }

        virtual omis_handle_t get_default_value(omis_handle_t type) override {
            auto ty = _Ty(type);
            if (ty->isIntegerTy()) {
                auto bits = ty->getIntegerBitWidth();
                return llvm::ConstantInt::get(context, llvm::APInt(bits, 0));
            }
            if (ty->isFloatingPointTy()) {
                return llvm::ConstantFP::get(context, llvm::APFloat(0.0f));
            }
            return llvm::ConstantPointerNull::get(llvm::Type::getVoidTy(context)->getPointerTo());
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

        virtual omis_handle_t value_func(omis_handle_t mod, const String& name, omis_handle_t type) override {
            llvm::Module* module = _Mod(mod);
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

        virtual void set_value_name(omis_handle_t value, const String& name) override {
            return _Val(value)->setName(name.cstr());
        }

        virtual omis_handle_t create_block(omis_handle_t func, const String& name) override {
            return llvm::BasicBlock::Create(context, name.cstr(), _Func(func));
        }

        virtual omis_handle_t get_active_block() override {
            auto block = IR.GetInsertBlock();
            return block;
        }

        virtual void set_active_block(omis_handle_t block) override {
            IR.SetInsertPoint(_Block(block));
        }

        virtual omis_handle_t get_block_tail(omis_handle_t block) override {
			auto blk = _Block(block);
            auto back = blk->empty() ? nullptr : &blk->back();
            return back;
        }

        virtual bool is_terminator_ins(omis_handle_t ins) override {
            return _Ins(ins)->isTerminator();
        }

        virtual omis_handle_t alloc(omis_handle_t type, const String& name) override {
            return IR.CreateAlloca(_Ty(type), nullptr, name.cstr());
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
            if(value == nullptr)
                return IR.CreateRetVoid();
            else
                return IR.CreateRet(_Val(value));
        }

        virtual omis_handle_t bitcast(omis_handle_t value, omis_handle_t type) override {
            return IR.CreateBitCast(_Val(value), _Ty(type));
        }

        virtual omis_handle_t get_ptr_val(omis_handle_t ptr) override {
            auto value = _Val(ptr);
            llvm::Type* type = value->getType();
            while (type->isPointerTy()) {
                if (llvm::isa<llvm::Function>(value))
                    break;
                if (type->getPointerElementType()->isFunctionTy())
                    break;
                if (type->getPointerElementType()->isStructTy())
                    break;
                if (type->getPointerElementType()->isArrayTy())
                    break;
                value = IR.CreateLoad(value);
                type = value->getType();
            }

            return value;
        }

        virtual omis_handle_t get_ptr_ref(omis_handle_t ptr) override {
            auto value = _Val(ptr);

            llvm::Type* type = value->getType();

            while (type->isPointerTy() && type->getPointerElementType()->isPointerTy()) {
                value = IR.CreateLoad(value);
                type = value->getType();
            }

            return value;
        }

        // virtual omis_handle_t make(omis_handle_t type, omis_handle_t count) = 0;
        // virtual void drop(omis_handle_t ptr) = 0;

        virtual bool jit(omis_handle_t mod) override {
            llvm::InitializeNativeTarget();
            llvm::InitializeNativeTargetAsmPrinter();
            llvm::InitializeNativeTargetAsmParser();

            auto* module = _Mod(mod);

            auto ee = llvm::EngineBuilder(std::unique_ptr<llvm::Module>(module))
                    .setEngineKind(llvm::EngineKind::JIT)
                    .create();

            ee->finalizeObject();

            llvm::Function* func = module->getFunction("$main");
            if(func == nullptr)
                return false;

            std::vector<llvm::GenericValue> args;
            auto retval = ee->runFunction(func, args);
            llvm::SmallString<32> str;
            retval.IntVal.toString(str, 10, true);
            printf("RET: %s \n", str.c_str());

            return true;
        }

        virtual bool aot(omis_handle_t mod) override {
            llvm::InitializeAllTargetInfos();
            llvm::InitializeAllTargets();
            llvm::InitializeAllTargetMCs();
            llvm::InitializeAllAsmParsers();
            llvm::InitializeAllAsmPrinters();

            auto* module = _Mod(mod);

            auto targetTriple = llvm::sys::getDefaultTargetTriple();
            std::string error;
            auto target = llvm::TargetRegistry::lookupTarget(targetTriple, error);

            auto CPU = "generic";
            auto features = "";
            llvm::TargetOptions opt;
            auto RM = llvm::Optional<llvm::Reloc::Model>();
            auto targetMachine = target->createTargetMachine(targetTriple, CPU, features, opt, RM);

            module->setDataLayout(targetMachine->createDataLayout());
            module->setTargetTriple(targetTriple);

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

            pass.run(*module);
            dest.flush();

            return true;
        }
    };

    omis_bridge_t* llvm_init() {
        return new llvm_bridge_t();
    }

    void llvm_quit(omis_bridge_t* bridge) {
        _DeletePointer(bridge);
    }
}
