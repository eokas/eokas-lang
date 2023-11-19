#include "./coder.h"
#include "./scope.h"
#include "./models.h"
#include "./x-module-core.h"
#include "./x-module-cstd.h"

#include <llvm/ADT/APFloat.h>
#include <llvm/ADT/STLExtras.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Verifier.h>

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace eokas {
    struct llvm_coder_t : llvm_module_t {
        llvm_function_t *func;
        llvm::BasicBlock *continuePoint;
        llvm::BasicBlock *breakPoint;

        explicit llvm_coder_t(llvm::LLVMContext &context, const String &name)
                : llvm_module_t(context, name) {
            this->func = nullptr;
            this->continuePoint = nullptr;
            this->breakPoint = nullptr;
        }

        virtual ~llvm_coder_t() {
        }

        void push_scope(llvm_function_t *f = nullptr) {
            this->scope = this->scope->add_child(f);
        }

        void pop_scope() {
            this->scope = this->scope->parent;
        }

        bool encode_module(ast_node_module_t *node) {
            if (node == nullptr)
                return false;

            auto *retT = type_i32;
            std::vector<llvm::Type *> argsT;

           auto mainFunc = this->create_function("$main", retT, argsT, false);
            this->scope->add_value_symbol("$main", mainFunc);

            this->func = mainFunc;

            llvm::IRBuilder<> &IR = mainFunc->IR;

            this->push_scope(mainFunc);
            {
                auto entry = mainFunc->add_basic_block("entry");
                IR.SetInsertPoint(entry);

                for (auto &stmt: node->entry->body) {
                    if (!this->encode_stmt(stmt))
                        return false;
                }

                mainFunc->add_tail_ret();
            }
            this->pop_scope();

            return true;
        }

        llvm_type_t *encode_type_ref(ast_node_type_t *node) {
            if (node == nullptr) {
                printf("Type node is null. \n");
                return nullptr;
            }

            const String &name = node->name;

            // TODO: implements array by Generic-Type
            if (name == "Array") {
                auto *elementT = this->encode_type_ref(node->args[0]);
                if (elementT == nullptr) {
                    printf("Invalid array element type. \n");
                    return nullptr;
                }

                auto* arrayType = this->create_type<llvm_type_array_t>();
                arrayType->set_element_type(elementT);

                return arrayType;
            }

            auto *symbol = this->scope->get_type_symbol(name, true);
            if (symbol == nullptr) {
                printf("The type '%s' is undefined.\n", name.cstr());
                return nullptr;
            }

            /* TODO: Support Generic Types
            if (type->generics.size() != node->args.size()) {
                printf("The generic type defination is not matched with the arguments. \n");
                return nullptr;
            }
            if (type->generics.size() > 0) {
				std::vector<llvm_type_t*> typeArgs = {};
				for(auto& arg : node->args)
				{
					auto* ty = this->encode_type_ref(arg);
					if(ty == nullptr)
					{
						printf("Type Args is invalid.");
						return nullptr;
					}
					typeArgs.push_back(ty);
				}
				
				return type->resolve(typeArgs);
            }
            */

            return symbol->type;
        }

        llvm::Value *encode_expr(ast_node_expr_t *node) {
            if (node == nullptr)
                return nullptr;

            switch (node->category) {
                case ast_category_t::EXPR_TRINARY:
                    return this->encode_expr_trinary(dynamic_cast<ast_node_expr_trinary_t *>(node));
                case ast_category_t::EXPR_BINARY:
                    return this->encode_expr_binary(dynamic_cast<ast_node_expr_binary_t *>(node));
                case ast_category_t::EXPR_UNARY:
                    return this->encode_expr_unary(dynamic_cast<ast_node_expr_unary_t *>(node));
                case ast_category_t::LITERAL_INT:
                    return this->encode_expr_int(dynamic_cast<ast_node_literal_int_t *>(node));
                case ast_category_t::LITERAL_FLOAT:
                    return this->encode_expr_float(dynamic_cast<ast_node_literal_float_t *>(node));
                case ast_category_t::LITERAL_BOOL:
                    return this->encode_expr_bool(dynamic_cast<ast_node_literal_bool_t *>(node));
                case ast_category_t::LITERAL_STRING:
                    return this->encode_expr_string(dynamic_cast<ast_node_literal_string_t *>(node));
                case ast_category_t::SYMBOL_REF:
                    return this->encode_expr_symbol_ref(dynamic_cast<ast_node_symbol_ref_t *>(node));
                case ast_category_t::FUNC_DEF:
                    return this->encode_expr_func_def(dynamic_cast<ast_node_func_def_t *>(node));
                case ast_category_t::FUNC_REF:
                    return this->encode_expr_func_ref(dynamic_cast<ast_node_func_ref_t *>(node));
                case ast_category_t::ARRAY_DEF:
                    return this->encode_expr_array_def(dynamic_cast<ast_node_array_def_t *>(node));
                case ast_category_t::ARRAY_REF:
                    return this->encode_expr_index_ref(dynamic_cast<ast_node_array_ref_t *>(node));
                case ast_category_t::OBJECT_DEF:
                    return this->encode_expr_object_def(dynamic_cast<ast_node_object_def_t *>(node));
                case ast_category_t::OBJECT_REF:
                    return this->encode_expr_object_ref(dynamic_cast<ast_node_object_ref_t *>(node));
                default:
                    return nullptr;
            }
            return nullptr;
        }

        llvm::Value *encode_expr_trinary(struct ast_node_expr_trinary_t *node) {
            if (node == nullptr)
                return nullptr;

            llvm_function_t *func = this->func;
            llvm::IRBuilder<> &IR = this->func->IR;

            llvm::BasicBlock *trinary_begin = func->add_basic_block("trinary.begin");
            llvm::BasicBlock *trinary_true = func->add_basic_block("trinary.true");
            llvm::BasicBlock *trinary_false = func->add_basic_block("trinary.false");
            llvm::BasicBlock *trinary_end = func->add_basic_block("trinary.end");

            IR.CreateBr(trinary_begin);
            IR.SetInsertPoint(trinary_begin);
            llvm::Value *cond = this->encode_expr(node->cond);
            if (cond == nullptr)
                return nullptr;
            cond = func->get_value(cond);
            if (!cond->getType()->isIntegerTy(1)) {
                printf("Condition must be a bool value.\n");
                return nullptr;
            }

            IR.CreateCondBr(cond, trinary_true, trinary_false);

            IR.SetInsertPoint(trinary_true);
            llvm::Value *trueV = this->encode_expr(node->branch_true);
            if (trueV == nullptr)
                return nullptr;
            IR.CreateBr(trinary_end);

            IR.SetInsertPoint(trinary_false);
            llvm::Value *falseV = this->encode_expr(node->branch_false);
            if (falseV == nullptr)
                return nullptr;
            IR.CreateBr(trinary_end);

            IR.SetInsertPoint(trinary_end);
            if (trueV->getType() != falseV->getType()) {
                printf("Type of true-branch must be the same as false-branch.\n");
                return nullptr;
            }

            llvm::PHINode *phi = IR.CreatePHI(trueV->getType(), 2);
            phi->addIncoming(trueV, trinary_true);
            phi->addIncoming(falseV, trinary_false);

            return phi;
        }

        llvm::Value *encode_expr_binary(ast_node_expr_binary_t *node) {
            if (node == nullptr)
                return nullptr;

            auto left = this->encode_expr(node->left);
            auto right = this->encode_expr(node->right);
            if (left == nullptr || right == nullptr)
                return nullptr;

            auto lhs = this->func->get_value(left);
            auto rhs = this->func->get_value(right);

            switch (node->op) {
                case ast_binary_oper_t::OR:
                case ast_binary_oper_t::AND:
                    return this->encode_expr_binary_logic(node->op, lhs, rhs);
                case ast_binary_oper_t::EQ:
                case ast_binary_oper_t::NE:
                case ast_binary_oper_t::LE:
                case ast_binary_oper_t::GE:
                case ast_binary_oper_t::LT:
                case ast_binary_oper_t::GT:
                    return this->encode_expr_binary_cmp(node->op, lhs, rhs);
                case ast_binary_oper_t::ADD:
                case ast_binary_oper_t::SUB:
                case ast_binary_oper_t::MUL:
                case ast_binary_oper_t::DIV:
                case ast_binary_oper_t::MOD:
                    return this->encode_expr_binary_arith(node->op, lhs, rhs);
                case ast_binary_oper_t::BIT_AND:
                case ast_binary_oper_t::BIT_OR:
                case ast_binary_oper_t::BIT_XOR:
                case ast_binary_oper_t::SHIFT_L:
                case ast_binary_oper_t::SHIFT_R:
                    return this->encode_expr_binary_bits(node->op, lhs, rhs);
                default:
                    return nullptr;
            }
        }

        llvm::Value *encode_expr_binary_logic(ast_binary_oper_t op, llvm::Value *lhs, llvm::Value *rhs) {
            using ins_type_t = std::function<llvm::Value *(llvm::IRBuilder<> &IR, llvm::Value *LHS, llvm::Value *RHS,
                                                           const llvm::Twine &Name)>;
            using func_type_t = llvm::Value *(llvm::IRBuilder<>::*)(llvm::Value *LHS, llvm::Value *RHS,
                                                                    const llvm::Twine &Name);

            static std::map<ast_binary_oper_t, ins_type_t> ins =
                    {
                            {ast_binary_oper_t::AND, (func_type_t) &llvm::IRBuilder<>::CreateAnd},
                            {ast_binary_oper_t::OR,  (func_type_t) &llvm::IRBuilder<>::CreateOr},
                    };

            auto &IR = this->func->IR;

            auto ltype = lhs->getType();
            auto rtype = rhs->getType();

            if (ltype->isIntegerTy(1) && rtype->isIntegerTy(1)) {
                return ins[op](IR, lhs, rhs, "");
            }

            printf("LHS or RHS is not bool value. \n");
            return nullptr;
        }

        llvm::Value *encode_expr_binary_cmp(ast_binary_oper_t op, llvm::Value *lhs, llvm::Value *rhs) {
            using ins_type_t = std::function<
                    llvm::Value *(llvm::IRBuilder<> &IR,
                                  llvm::CmpInst::Predicate Predicate,
                                  llvm::Value *LHS,
                                  llvm::Value *RHS,
                                  const llvm::Twine &Name,
                                  llvm::MDNode *FPMathTag)>;

            static ins_type_t ins = &llvm::IRBuilder<>::CreateCmp;

            static std::map<ast_binary_oper_t, llvm::CmpInst::Predicate> op_i =
                    {
                            {ast_binary_oper_t::EQ, llvm::CmpInst::ICMP_EQ},
                            {ast_binary_oper_t::NE, llvm::CmpInst::ICMP_NE},
                            {ast_binary_oper_t::LE, llvm::CmpInst::ICMP_SLE},
                            {ast_binary_oper_t::GE, llvm::CmpInst::ICMP_SGE},
                            {ast_binary_oper_t::LT, llvm::CmpInst::ICMP_SLT},
                            {ast_binary_oper_t::GT, llvm::CmpInst::ICMP_SGT},
                    };

            static std::map<ast_binary_oper_t, llvm::CmpInst::Predicate> op_f =
                    {
                            {ast_binary_oper_t::EQ, llvm::CmpInst::FCMP_OEQ},
                            {ast_binary_oper_t::NE, llvm::CmpInst::FCMP_ONE},
                            {ast_binary_oper_t::LE, llvm::CmpInst::FCMP_OLE},
                            {ast_binary_oper_t::GE, llvm::CmpInst::FCMP_OGE},
                            {ast_binary_oper_t::LT, llvm::CmpInst::FCMP_OLT},
                            {ast_binary_oper_t::GT, llvm::CmpInst::FCMP_OGT},
                    };

            auto &IR = this->func->IR;
            auto ltype = lhs->getType();
            auto rtype = rhs->getType();

            if (ltype->isIntegerTy() && rtype->isIntegerTy())
                return ins(IR, op_i[op], lhs, rhs, "", nullptr);

            if (ltype->isFloatingPointTy() && rtype->isFloatingPointTy())
                return ins(IR, op_f[op], lhs, rhs, "", nullptr);

            if (ltype->isPointerTy() && rtype->isPointerTy()) {
                lhs = IR.CreatePtrToInt(lhs, llvm::Type::getInt64Ty(context));
                rhs = IR.CreatePtrToInt(rhs, llvm::Type::getInt64Ty(context));
                return ins(IR, op_i[op], lhs, rhs, "", nullptr);
            }

            if (ltype->isIntegerTy() && rtype->isFloatingPointTy()) {
                lhs = IR.CreateSIToFP(lhs, llvm::Type::getDoubleTy(context));
                return ins(IR, op_f[op], lhs, rhs, "", nullptr);
            }

            if (ltype->isFloatingPointTy() && rtype->isIntegerTy()) {
                rhs = IR.CreateSIToFP(rhs, llvm::Type::getDoubleTy(context));
                return ins(IR, op_f[op], lhs, rhs, "", nullptr);
            }

            printf("Type of LHS or RHS is invalid.\n");
            return nullptr;
        }

        llvm::Value *encode_expr_binary_arith(ast_binary_oper_t op, llvm::Value *lhs, llvm::Value *rhs) {
            using ins_type_t = std::function<llvm::Value *(llvm::IRBuilder<> &IR, llvm::Value *LHS, llvm::Value *RHS)>;
            using func_type_t = llvm::Value *(llvm::IRBuilder<>::*)(llvm::Value *LHS, llvm::Value *RHS);

            static std::map<ast_binary_oper_t, ins_type_t> ins_i = {
                    {ast_binary_oper_t::ADD, (func_type_t) &llvm::IRBuilder<>::CreateAdd},
                    {ast_binary_oper_t::SUB, (func_type_t) &llvm::IRBuilder<>::CreateSub},
                    {ast_binary_oper_t::MUL, (func_type_t) &llvm::IRBuilder<>::CreateMul},
                    {ast_binary_oper_t::DIV, (func_type_t) &llvm::IRBuilder<>::CreateSDiv},
                    {ast_binary_oper_t::MOD, (func_type_t) &llvm::IRBuilder<>::CreateSRem},
            };

            static std::map<ast_binary_oper_t, ins_type_t> ins_f = {
                    {ast_binary_oper_t::ADD, (func_type_t) &llvm::IRBuilder<>::CreateFAdd},
                    {ast_binary_oper_t::SUB, (func_type_t) &llvm::IRBuilder<>::CreateFSub},
                    {ast_binary_oper_t::MUL, (func_type_t) &llvm::IRBuilder<>::CreateFMul},
                    {ast_binary_oper_t::DIV, (func_type_t) &llvm::IRBuilder<>::CreateFDiv},
                    {ast_binary_oper_t::MOD, (func_type_t) &llvm::IRBuilder<>::CreateFRem},
            };

            auto &IR = this->func->IR;
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

        llvm::Value *encode_expr_binary_bits(ast_binary_oper_t op, llvm::Value *lhs, llvm::Value *rhs) {
            auto &IR = this->func->IR;
            auto ltype = lhs->getType();
            auto rtype = rhs->getType();

            if (ltype->isIntegerTy() && rtype->isIntegerTy()) {
                if (op == ast_binary_oper_t::BIT_AND) return IR.CreateAnd(lhs, rhs);
                if (op == ast_binary_oper_t::BIT_OR) return IR.CreateOr(lhs, rhs);
                if (op == ast_binary_oper_t::BIT_XOR) return IR.CreateXor(lhs, rhs);
                if (op == ast_binary_oper_t::SHIFT_L) return IR.CreateShl(lhs, rhs);

                // CreateLShr: 逻辑右移：在左边补 0
                // CreateShr: 算术右移：在左边补 符号位
                // 我们采用逻辑右移
                if (op == ast_binary_oper_t::SHIFT_R) return IR.CreateLShr(lhs, rhs);
            }

            printf("Type of LHS or RHS is invalid.\n");
            return nullptr;
        }

        llvm::Value *encode_expr_unary(ast_node_expr_unary_t *node) {
            if (node == nullptr)
                return nullptr;

            auto right = this->encode_expr(node->right);
            if (right == nullptr)
                return nullptr;

            auto rhs = this->func->get_value(right);

            switch (node->op) {
                case ast_unary_oper_t::POS:
                    return rhs;
                case ast_unary_oper_t::NEG:
                    return this->encode_expr_unary_neg(rhs);
                case ast_unary_oper_t::NOT:
                    return this->encode_expr_unary_not(rhs);
                case ast_unary_oper_t::FLIP:
                    return this->encode_expr_unary_flip(rhs);
                default:
                    return nullptr;
            }
        }

        llvm::Value *encode_expr_unary_neg(llvm::Value *rhs) {
            auto rtype = rhs->getType();

            if (rtype->isIntegerTy())
                return this->func->IR.CreateNeg(rhs);

            if (rtype->isFloatingPointTy())
                return this->func->IR.CreateFNeg(rhs);

            printf("Type of RHS is invalid.\n");
            return nullptr;
        }

        llvm::Value *encode_expr_unary_not(llvm::Value *rhs) {
            auto rtype = rhs->getType();

            if (rtype->isIntegerTy() && rtype->getIntegerBitWidth() == 1)
                return this->func->IR.CreateNot(rhs);

            printf("Type of RHS is invalid.\n");
            return nullptr;
        }

        llvm::Value *encode_expr_unary_flip(llvm::Value *rhs) {
            auto rtype = rhs->getType();

            if (rtype->isIntegerTy()) {
                return this->func->IR.CreateXor(rhs, llvm::ConstantInt::get(rtype,
                                                                            llvm::APInt(rtype->getIntegerBitWidth(),
                                                                                        0xFFFFFFFF)));
            }
            printf("Type of RHS is invalid.\n");
            return nullptr;
        }

        llvm::Value *encode_expr_int(ast_node_literal_int_t *node) {
            if (node == nullptr)
                return nullptr;

            u64_t vals = *((u64_t *) &(node->value));
            u32_t bits = vals > 0xFFFFFFFF ? 64 : 32;

            return llvm::ConstantInt::get(context, llvm::APInt(bits, node->value));
        }

        llvm::Value *encode_expr_float(ast_node_literal_float_t *node) {
            if (node == nullptr)
                return nullptr;

            return llvm::ConstantFP::get(context, llvm::APFloat(node->value));
        }

        llvm::Value *encode_expr_bool(ast_node_literal_bool_t *node) {
            if (node == nullptr)
                return nullptr;
            return llvm::ConstantInt::getBool(context, node->value);
        }

        llvm::Value *encode_expr_string(ast_node_literal_string_t *node) {
            if (node == nullptr)
                return nullptr;

            auto str = func->string_make(node->value.cstr());
            return str;
        }

        llvm::Value *encode_expr_symbol_ref(ast_node_symbol_ref_t *node) {
            if (node == nullptr)
                return nullptr;

            auto *symbol = this->scope->get_value_symbol(node->name, true);
            if (symbol == nullptr) {
                printf("Symbol '%s' is undefined.\n", node->name.cstr());
                return nullptr;
            }

            // local-value-ref
            if (symbol->scope->func == this->func) {
                return symbol->value->value;
            }

            /*
            // up-value-ref
            {
                auto upvalStruct = this->upvals[this->func];
                int index = upvalStruct->get_member_index(node->name);
                if(index < 0)
                {
                    upvalStruct->add_member(node->name, symbol->type, symbol->value);
                    upvalStruct->resolve();
                    index = upvalStruct->get_member_index(node->name);
                }
                if(index < 0)
                {
                    printf("Create up-value '%s' failed.\n", node->name.cstr());
                    return nullptr;
                }

                auto arg0 = this->func->getArg(0);
                auto ptr = this->func->IR.CreateConstGEP2_32(arg0->getType()->getPointerElementType(), arg0, 0, index);
                return ptr;
            }
            */

            return symbol->value->value;
        }

        llvm::Value *encode_expr_func_def(ast_node_func_def_t *node) {
            if (node == nullptr)
                return nullptr;

            auto *retType = this->encode_type_ref(node->rtype);
            if (retType == nullptr)
                return nullptr;

            std::vector<llvm::Type *> argTypes;
            for (auto &arg: node->args) {
                auto *argType = this->encode_type_ref(arg.type);
                if (argType == nullptr)
                    return nullptr;
                if (argType->is_reference_type())
                    argTypes.push_back(argType->handle->getPointerTo());
                else
                    argTypes.push_back(argType->handle);
            }

            auto newFunc = this->create_function("", retType->handle, argTypes, false);

            auto oldFunc = this->func;
            auto oldIB = this->func->IR.GetInsertBlock();

            this->func = newFunc;

            this->push_scope(newFunc);
            {
                llvm::BasicBlock *entry = newFunc->add_basic_block("entry");
                this->func->IR.SetInsertPoint(entry);

                // self
                auto self = newFunc;
                this->scope->add_value_symbol("self", self);

                // args
                for (size_t index = 0; index < node->args.size(); index++) {
                    const char *name = node->args.at(index).name.cstr();
                    auto arg = newFunc->handle->getArg(index + 1);
                    arg->setName(name);

                    if (!this->scope->add_value_symbol(name, new llvm_value_t(this, arg))) {
                        printf("The symbol name '%s' is already existed.\n", name);
                        return nullptr;
                    }
                }

                // body
                for (auto &stmt: node->body) {
                    if (!this->encode_stmt(stmt))
                        return nullptr;
                }
                auto &lastOp = this->func->IR.GetInsertBlock()->back();
                if (!lastOp.isTerminator()) {
                    if (newFunc->handle->getReturnType()->isVoidTy())
                        this->func->IR.CreateRetVoid();
                    else
                        this->func->IR.CreateRet(this->get_default_value(retType->handle));
                }
            }
            this->pop_scope();

            this->func = oldFunc;
            this->func->IR.SetInsertPoint(oldIB);

            return newFunc->handle;
        }

        llvm::Value *encode_expr_func_ref(ast_node_func_ref_t *node) {
            if (node == nullptr)
                return nullptr;

            llvm::Value *funcExpr = this->encode_expr(node->func);
            if (funcExpr == nullptr) {
                printf("The function is undefined.\n");
                return nullptr;
            }

            auto funcPtr = this->func->get_value(funcExpr);
            llvm::FunctionType *funcType = nullptr;
            if (funcPtr->getType()->isFunctionTy()) {
                funcType = llvm::cast<llvm::FunctionType>(funcPtr->getType());
            } else if (funcPtr->getType()->isPointerTy() &&
                       funcPtr->getType()->getPointerElementType()->isFunctionTy()) {
                funcType = llvm::cast<llvm::FunctionType>(funcPtr->getType()->getPointerElementType());
            } else {
                printf("Invalid function type.\n");
                return nullptr;
            }

            std::vector<llvm::Value *> args;
            for (auto i = 0; i < node->args.size(); i++) {
                auto *paramT = funcType->getParamType(i);
                auto *argV = this->encode_expr(node->args.at(i));
                if (argV == nullptr)
                    return nullptr;

                argV = this->func->get_value(argV);

                if (paramT != argV->getType()) {
                    if (paramT == this->type_cstr) {
                        argV = func->cstr_from_value(argV);
                    }
                    if (paramT == this->type_string_ptr) {
                        argV = func->string_from_value(argV);
                    } else if (!argV->getType()->canLosslesslyBitCastTo(paramT)) {
                        printf("The type of param[%d] can't cast to the param type of function.\n", i);
                        return nullptr;
                    }
                }

                args.push_back(argV);
            }

            llvm::Value *retval = this->func->IR.CreateCall(funcType, funcPtr, args);
            return retval;
        }

        llvm::Value *encode_expr_array_def(ast_node_array_def_t *node) {
            if (node == nullptr)
                return nullptr;

            std::vector<llvm::Value *> arrayElements;
            llvm::Type *arrayElementType = nullptr;
            for (auto element: node->elements) {
                auto elementV = this->encode_expr(element);
                if (elementV == nullptr)
                    return nullptr;
                elementV = this->func->get_value(elementV);

                auto elementT = elementV->getType();
                if (arrayElementType == nullptr) {
                    arrayElementType = elementT;
                } else if (arrayElementType != elementT) {
                    printf("The type of some elements is not same as others.\n");
                    return nullptr;
                }

                arrayElements.push_back(elementV);
            }

            if (arrayElements.empty() || arrayElementType == nullptr)
                arrayElementType = llvm::Type::getInt32Ty(context);

            auto arrayT = this->create_type<llvm_type_array_t>();
            arrayT->set_element_type(this->get_type_symbol(arrayElementType)->type);

            auto arrayP = this->func->make(arrayT);
            this->func->array_set(arrayP, arrayElements);

            return arrayP;
        }

        llvm::Value *encode_expr_index_ref(ast_node_array_ref_t *node) {
            if (node == nullptr)
                return nullptr;

            auto objV = this->encode_expr(node->obj);
            auto keyV = this->encode_expr(node->key);
            if (objV == nullptr || keyV == nullptr)
                return nullptr;

            objV = this->func->get_value(objV);
            keyV = this->func->get_value(keyV);
            auto objT = objV->getType();
            auto keyT = keyV->getType();

            if (module->is_array(objT)) {
                if (keyT->isIntegerTy()) {
                    return this->func->array_get(objV, keyV);
                } else {
                    printf("The type of index is invalid.\n");
                    return nullptr;
                }
            } else if (objT == module->type_string_ptr) {
                if (keyT->isIntegerTy()) {
                    return this->func->string_get_char(objV, keyV);
                } else {
                    printf("The type of index is invalid.\n");
                    return nullptr;
                }
            } else {
                printf("Index-Access is not defined on the object.\n");
                return nullptr;
            }
        }

        llvm::Value *encode_expr_object_def(ast_node_object_def_t *node) {
            if (node == nullptr)
                return nullptr;

            auto *structType = this->encode_type_ref(node->type);
            if (structType == nullptr)
                return nullptr;

            auto *structInfo = module->get_struct(structType);
            if (structInfo == nullptr)
                return nullptr;

            for (auto &objectMember: node->members) {
                auto structMember = structInfo->get_member(objectMember.first);
                if (structMember == nullptr) {
                    printf("Object member '%s' is not defined in struct.\n", objectMember.first.cstr());
                    return nullptr;
                }
                // todo: check object-member-type equals to struct-member-type.
            }

            // make object instance
            llvm::Value *instance = this->func->make(structType);

            for (u32_t index = 0; index < structInfo->members.size(); index++) {
                auto &structMember = structInfo->members.at(index);
                auto objectMember = node->members.find(structMember->name);
                llvm::Value *memV = nullptr;
                if (objectMember != node->members.end()) {
                    memV = this->encode_expr(objectMember->second);
                    if (memV == nullptr)
                        return nullptr;
                    memV = this->func->get_value(memV);
                } else {
                    memV = structMember->value != nullptr ? structMember->value : this->get_default_value(
                            structMember->type);
                }

                if (memV) {
                    String memN = String::format("this.%s", structMember->name.cstr());
                    llvm::Value *memP = this->func->IR.CreateStructGEP(structType, instance, index, memN.cstr());
                    this->func->IR.CreateStore(memV, memP);
                }
            }

            return instance;
        }

        llvm::Value *encode_expr_object_ref(ast_node_object_ref_t *node) {
            if (node == nullptr)
                return nullptr;

            auto objV = this->encode_expr(node->obj);
            auto keyV = this->func->IR.CreateGlobalString(node->key.cstr());
            if (objV == nullptr || keyV == nullptr)
                return nullptr;

            objV = this->func->get_value(objV);
            auto objT = objV->getType();
            if (!(objT->isPointerTy() && objT->getPointerElementType()->isStructTy())) {
                printf("The value is not a object reference.\n");
                return nullptr;
            }

            auto structInfo = module->get_struct(objT->getPointerElementType());
            if (structInfo == nullptr) {
                printf("Can't find the type of this value.\n");
                return nullptr;
            }


            auto index = structInfo->get_member_index(node->key);
            if (index == -1) {
                printf("The object doesn't have a member named '%s'. \n", node->key.cstr());
                return nullptr;
            }

            auto structType = structInfo->type;
            llvm::Value *value = this->func->IR.CreateStructGEP(structType, objV, index);
            return value;
        }

        bool encode_stmt(ast_node_stmt_t *node) {
            if (node == nullptr)
                return false;

            switch (node->category) {
                case ast_category_t::STRUCT_DEF:
                    return this->encode_stmt_struct_def(dynamic_cast<ast_node_struct_def_t *>(node));
                case ast_category_t::ENUM_DEF:
                    return this->encode_stmt_enum_def(dynamic_cast<ast_node_enum_def_t *>(node));
                case ast_category_t::PROC_DEF:
                    return this->encode_stmt_proc_def(dynamic_cast<ast_node_proc_def_t *>(node));
                case ast_category_t::SYMBOL_DEF:
                    return this->encode_stmt_symbol_def(dynamic_cast<ast_node_symbol_def_t *>(node));
                case ast_category_t::BREAK:
                    return this->encode_stmt_break(dynamic_cast<ast_node_break_t *>(node));
                case ast_category_t::CONTINUE:
                    return this->encode_stmt_continue(dynamic_cast<ast_node_continue_t *>(node));
                case ast_category_t::RETURN:
                    return this->encode_stmt_return(dynamic_cast<ast_node_return_t *>(node));
                case ast_category_t::IF:
                    return this->encode_stmt_if(dynamic_cast<ast_node_if_t *>(node));
                case ast_category_t::LOOP:
                    return this->encode_stmt_loop(dynamic_cast<ast_node_loop_t *>(node));
                case ast_category_t::BLOCK:
                    return this->encode_stmt_block(dynamic_cast<ast_node_block_t *>(node));
                case ast_category_t::ASSIGN:
                    return this->encode_stmt_assign(dynamic_cast<ast_node_assign_t *>(node));
                case ast_category_t::INVOKE:
                    return this->encode_stmt_invoke(dynamic_cast<ast_node_invoke_t *>(node));
                default:
                    return false;
            }

            return false;
        }

        bool encode_stmt_struct_def(ast_node_struct_def_t *node) {
            if (node == nullptr)
                return false;

            auto *thisInstanceInfo = module->new_struct(node->name);

            for (const auto &thisMember: node->members) {
                const String &memName = thisMember.name;
                if (thisInstanceInfo->get_member(memName) != nullptr) {
                    printf("The member named '%s' is already exists.\n", memName.cstr());
                    return false;
                }

                auto memType = this->encode_type_ref(thisMember.type);
                if (memType == nullptr)
                    return false;

                auto memValue = this->encode_expr(thisMember.value);
                if (memValue == nullptr) {
                    memValue = this->get_default_value(memType);
                }

                thisInstanceInfo->add_member(memName, memType, memValue);
            }

            thisInstanceInfo->resolve();
            if (!this->scope->add_type_symbol(thisInstanceInfo->name, thisInstanceInfo->type)) {
                printf("There is a same schema named %s in this scope.\n", thisInstanceInfo->name.cstr());
                return false;
            }

            return true;
        }

        bool encode_stmt_enum_def(struct ast_node_enum_def_t *node) {
            if (node == nullptr)
                return false;

            const String name = node->name;

            auto *thisInstanceInfo = this->func->new_struct(node->name);
            thisInstanceInfo->add_member("value", this->type_i32);
            thisInstanceInfo->resolve();

            const String staticTypePrefix = "$_Static";
            auto *thisStaticInfo = module->new_struct(staticTypePrefix + node->name);
            for (const auto &thisMember: node->members) {
                auto memName = thisMember.first;
                if (thisStaticInfo->get_member(memName) != nullptr) {
                    printf("The member named '%s' is already exists.\n", memName.cstr());
                    return false;
                }

                auto v = this->func->IR.getInt32(thisMember.second);
                auto o = this->func->IR.CreateAlloca(thisInstanceInfo->type);
                auto n = String::format("%s.%s", node->name.cstr(), memName.cstr());
                auto p = this->func->IR.CreateStructGEP(o, 0, n.cstr());
                this->func->IR.CreateStore(v, p);
                auto memValue = o;

                thisStaticInfo->add_member(memName, thisInstanceInfo->type, memValue);
            }
            thisStaticInfo->resolve();

            if (!this->scope->add_type_symbol(thisStaticInfo->name, thisStaticInfo->type) ||
                !this->scope->add_type_symbol(thisInstanceInfo->name, thisInstanceInfo->type)) {
                printf("There is a same schema named %s in this scope.\n", thisInstanceInfo->name.cstr());
                return false;
            }

            // make static object
            llvm::Value *staticV = this->func->make(this->scope->func, this->func->IR, thisStaticInfo->type);
            for (u32_t index = 0; index < thisStaticInfo->members.size(); index++) {
                auto &mem = thisStaticInfo->members.at(index);
                auto memV = this->func->IR.CreateLoad(mem->value);

                String memN = String::format("%s.%s", name.cstr(), mem->name.cstr());
                llvm::Value *memP = this->func->IR.CreateStructGEP(thisStaticInfo->type, staticV, index, memN.cstr());
                this->func->IR.CreateStore(memV, memP);
            }
            if (!this->scope->add_value_symbol(name, staticV)) {
                printf("There is a same symbol named %s in this scope.\n", name.cstr());
                return false;
            }

            return true;
        }

        bool encode_stmt_proc_def(ast_node_proc_def_t *node) {
            if (node == nullptr)
                return false;

            if (this->scope->get_type_symbol(node->name, false) != nullptr)
                return false;

            auto *retType = this->encode_type_ref(node->type);

            std::vector<llvm_type_t*> argTypes;
            for (auto arg: node->args) {
                auto *argType = this->encode_type_ref(arg.second);
                if (argType == nullptr)
                    return false;
                argTypes.push_back(argType);
            }

            auto retT = retType->handle;
            std::vector<llvm::Type*> argsT = {};
            for(auto& t : argTypes) {
                argsT.push_back(t->handle);
            }

            llvm::FunctionType *procT = llvm::FunctionType::get(retT, argsT, false);
            auto proc_type = this->create_type(procT->getPointerTo());
            this->scope->add_type_symbol(node->name, proc_type);

            return true;
        }

        bool encode_stmt_symbol_def(struct ast_node_symbol_def_t *node) {
            if (node == nullptr)
                return false;

            if (this->scope->get_value_symbol(node->name, false) != nullptr) {
                printf("The symbol '%s' is undefined.", node->name.cstr());
                return false;
            }

            auto type = this->encode_type_ref(node->type);
            auto expr = this->encode_expr(node->value);
            if (expr == nullptr)
                return false;

            auto value = this->func->get_value(expr);

            llvm::Type *stype = nullptr;
            llvm::Type* dtype = type != nullptr ? type->handle : nullptr;
            llvm::Type *vtype = value->getType();
            if (dtype != nullptr) {
                stype = dtype;

                // Check type compatibilities.
                do {
                    if (stype == vtype)
                        break;
                    if (vtype->canLosslesslyBitCastTo(stype))
                        break;
                    if (vtype->isPointerTy() && vtype->getPointerElementType() == stype) {
                        stype = dtype = vtype;
                        break;
                    }

                    // TODO: 校验类型合法性, 值类型是否遵循标记类型

                    printf("Type of value can not cast to the type of symbol.\n");
                    return false;
                }
                while (false);
            }
            else {
                stype = vtype;
            }

            if (stype->isVoidTy()) {
                printf("Void-Type can't assign to a symbol.\n");
                return false;
            }

            llvm::Value* symbol = this->func->define_local_var(node->name, stype, value);
            auto value_symbol = this->create_value(symbol);

            if (!scope->add_value_symbol(node->name, value_symbol)) {
                printf("There is a symbol named %s in this scope.\n", node->name.cstr());
                return false;
            }

            return true;
        }

        bool encode_stmt_break(struct ast_node_break_t *node) {
            if (node == nullptr)
                return false;

            if (this->breakPoint == nullptr)
                return false;

            this->func->IR.CreateBr(this->breakPoint);

            return true;
        }

        bool encode_stmt_continue(struct ast_node_continue_t *node) {
            if (node == nullptr)
                return false;

            if (this->continuePoint == nullptr)
                return false;

            this->func->IR.CreateBr(this->continuePoint);

            return true;
        }

        bool encode_stmt_return(struct ast_node_return_t *node) {
            if (node == nullptr)
                return false;

            auto expectedRetType = this->func->type->getReturnType();

            if (node->value == nullptr) {
                if (!expectedRetType->isVoidTy()) {
                    printf("The function must return a value.\n");
                    return false;
                }

                this->func->IR.CreateRetVoid();
                return true;
            }

            auto expr = this->encode_expr(node->value);
            if (expr == nullptr) {
                printf("Invalid ret value.\n");
                return false;
            }
            auto value = this->func->get_value(expr);
            auto actureRetType = value->getType();
            if (actureRetType != expectedRetType && !actureRetType->canLosslesslyBitCastTo(expectedRetType)) {
                printf("The type of return value can't cast to return type of function.\n");
                return false;
            }

            this->func->IR.CreateRet(value);

            return true;
        }

        bool encode_stmt_if(struct ast_node_if_t *node) {
            if (node == nullptr)
                return false;

            llvm::BasicBlock *if_true = this->func->add_basic_block("if.true");
            llvm::BasicBlock *if_false = this->func->add_basic_block("if.false");
            llvm::BasicBlock *if_end = this->func->add_basic_block("if.end");

            auto condV = this->encode_expr(node->cond);
            if (condV == nullptr)
                return false;
            condV = this->func->get_value(condV);
            if (!condV->getType()->isIntegerTy(1)) {
                printf("The label 'if.cond' need a bool value.\n");
                return false;
            }
            this->func->IR.CreateCondBr(condV, if_true, if_false);

            // if-true
            this->func->IR.SetInsertPoint(if_true);
            if (node->branch_true != nullptr) {
                if (!this->encode_stmt(node->branch_true))
                    return false;
                auto lastBlock = this->func->IR.GetInsertBlock();
                if (lastBlock != if_true && !lastBlock->back().isTerminator()) {
                    this->func->IR.CreateBr(if_end);
                }
            }
            if (!if_true->back().isTerminator()) {
                this->func->IR.CreateBr(if_end);
            }

            // if-false
            this->func->IR.SetInsertPoint(if_false);
            if (node->branch_false != nullptr) {
                if (!this->encode_stmt(node->branch_false))
                    return false;
                auto lastBlock = this->func->IR.GetInsertBlock();
                if (lastBlock != if_false && !lastBlock->back().isTerminator()) {
                    this->func->IR.CreateBr(if_end);
                }
            }
            if (!if_false->back().isTerminator()) {
                this->func->IR.CreateBr(if_end);
            }

            this->func->IR.SetInsertPoint(if_end);

            return true;
        }

        bool encode_stmt_loop(struct ast_node_loop_t *node) {
            if (node == nullptr)
                return false;

            this->push_scope();

            llvm::BasicBlock *loop_cond = this->func->add_basic_block("loop.cond");
            llvm::BasicBlock *loop_step = this->func->add_basic_block("loop.step");
            llvm::BasicBlock *loop_body = this->func->add_basic_block("loop.body");
            llvm::BasicBlock *loop_end = this->func->add_basic_block("loop.end");

            auto oldContinuePoint = this->continuePoint;
            auto oldBreakPoint = this->breakPoint;
            this->continuePoint = loop_step;
            this->breakPoint = loop_end;

            if (!this->encode_stmt(node->init))
                return false;
            this->func->IR.CreateBr(loop_cond);

            this->func->IR.SetInsertPoint(loop_cond);
            auto condV = this->encode_expr(node->cond);
            if (condV == nullptr)
                return false;
            condV = this->func->get_value(condV);
            if (!condV->getType()->isIntegerTy(1)) {
                printf("The label 'for.cond' need a bool value.\n");
                return false;
            }
            this->func->IR.CreateCondBr(condV, loop_body, loop_end);

            this->func->IR.SetInsertPoint(loop_body);
            if (node->body != nullptr) {
                if (!this->encode_stmt(node->body))
                    return false;
                auto &lastOp = loop_body->back();
                if (!lastOp.isTerminator()) {
                    this->func->IR.CreateBr(loop_step);
                }
            }
            if (!loop_body->back().isTerminator()) {
                this->func->IR.CreateBr(loop_step);
            }

            this->func->IR.SetInsertPoint(loop_step);
            if (!this->encode_stmt(node->step))
                return false;
            this->func->IR.CreateBr(loop_cond);

            this->func->IR.SetInsertPoint(loop_end);

            this->continuePoint = oldContinuePoint;
            this->breakPoint = oldBreakPoint;

            this->pop_scope();

            return true;
        }

        bool encode_stmt_block(struct ast_node_block_t *node) {
            if (node == nullptr)
                return false;

            this->push_scope();

            for (auto &stmt: node->stmts) {
                if (!this->encode_stmt(stmt))
                    return false;
            }

            this->pop_scope();

            return true;
        }

        bool encode_stmt_assign(struct ast_node_assign_t *node) {
            if (node == nullptr)
                return false;

            auto left = this->encode_expr(node->left);
            auto right = this->encode_expr(node->right);
            if (left == nullptr || right == nullptr)
                return false;

            auto ptr = this->func->ref_value(left);
            auto val = this->func->get_value(right);
            this->func->IR.CreateStore(val, ptr);

            return true;
        }

        bool encode_stmt_invoke(struct ast_node_invoke_t *node) {
            if (node == nullptr)
                return false;

            if (!this->encode_expr_func_ref(node->expr))
                return false;

            return true;
        }
    };

    llvm_module_t *llvm_encode(llvm::LLVMContext &context, ast_node_module_t *module) {
        auto *coder = new llvm_coder_t(context, module->name);
        if (!coder->encode_module(module)) {
            return nullptr;
        }
        return coder;
    }
}
