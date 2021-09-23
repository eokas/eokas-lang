#include "coder.h"
#include "runtime.h"

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

_BeginNamespace(eokas)

    struct llvm_scope_t {
        llvm_scope_t *parent;
        std::vector<llvm_scope_t *> children;

        std::map<String, llvm::Value *> symbols;
        std::map<String, llvm::Type *> types;
        std::map<String, ast_type_generic_t *> generics;

        llvm_scope_t(llvm_scope_t *parent)
                : parent(parent), children(), symbols(), types(), generics() {
        }

        virtual ~llvm_scope_t() {
            this->parent = nullptr;
            _DeleteList(this->children);
            this->types.clear();
            this->symbols.clear();
        }

        llvm_scope_t *addChild() {
            llvm_scope_t *child = new llvm_scope_t(this);
            this->children.push_back(child);
            return child;
        }

        llvm::Value *getSymbol(const String &name, bool lookUp) {
            if (lookUp) {
                for (auto scope = this; scope != nullptr; scope = scope->parent) {
                    auto iter = scope->symbols.find(name);
                    if (iter != scope->symbols.end())
                        return iter->second;
                }
                return nullptr;
            } else {
                auto iter = this->symbols.find(name);
                if (iter != this->symbols.end())
                    return iter->second;
                return nullptr;
            }
        }

        llvm::Type *getType(const String &name, bool lookUp) {
            if (lookUp) {
                for (auto scope = this; scope != nullptr; scope = scope->parent) {
                    auto iter = scope->types.find(name);
                    if (iter != scope->types.end())
                        return iter->second;
                }
                return nullptr;
            } else {
                auto iter = this->types.find(name);
                if (iter != this->types.end())
                    return iter->second;
                return nullptr;
            }
        }

        ast_type_generic_t *getGeneric(const String &name, bool lookUp) {
            if (lookUp) {
                for (auto scope = this; scope != nullptr; scope = scope->parent) {
                    auto iter = scope->generics.find(name);
                    if (iter != scope->generics.end())
                        return iter->second;
                }
                return nullptr;
            } else {
                auto iter = this->generics.find(name);
                if (iter != this->generics.end())
                    return iter->second;
                return nullptr;
            }
        }
    };

    struct llvm_coder_t {
        llvm::LLVMContext &llvm_context;
        llvm::IRBuilder<> llvm_builder;
        llvm::Module *llvm_module;
        llvm_scope_t *root;
        llvm_scope_t *scope;
        llvm::Function *func;
        llvm::BasicBlock *continuePoint;
        llvm::BasicBlock *breakPoint;

        std::map<llvm::Type *, ast_stmt_struct_def_t *> structs;

        llvm::Type *type_void;
        llvm::Type *type_i8;
        llvm::Type *type_i16;
        llvm::Type *type_i32;
        llvm::Type *type_i64;
        llvm::Type *type_u8;
        llvm::Type *type_u16;
        llvm::Type *type_u32;
        llvm::Type *type_u64;
        llvm::Type *type_f32;
        llvm::Type *type_f64;
        llvm::Type *type_bool;
        llvm::Type *type_string;

        llvm::Value *const_zero;

        explicit llvm_coder_t(llvm::LLVMContext &llvm_context)
                : llvm_context(llvm_context), llvm_builder(llvm_context), llvm_module(nullptr) {
            this->root = new llvm_scope_t(nullptr);
            this->scope = this->root;
            this->func = nullptr;
            this->continuePoint = nullptr;
            this->breakPoint = nullptr;

            type_void = llvm::Type::getVoidTy(llvm_context);
            type_i8 = llvm::Type::getInt8Ty(llvm_context);
            type_i16 = llvm::Type::getInt16Ty(llvm_context);
            type_i32 = llvm::Type::getInt32Ty(llvm_context);
            type_i64 = llvm::Type::getInt64Ty(llvm_context);
            type_u8 = llvm::Type::getInt8Ty(llvm_context);
            type_u16 = llvm::Type::getInt16Ty(llvm_context);
            type_u32 = llvm::Type::getInt32Ty(llvm_context);
            type_u64 = llvm::Type::getInt64Ty(llvm_context);
            type_f32 = llvm::Type::getFloatTy(llvm_context);
            type_f64 = llvm::Type::getDoubleTy(llvm_context);
            type_bool = llvm::Type::getInt1Ty(llvm_context);
            type_string = llvm::Type::getInt8PtrTy(llvm_context);

            const_zero = llvm::ConstantInt::get(llvm_context, llvm::APInt(32, 0));
        }

        virtual ~llvm_coder_t() {
            _DeletePointer(this->scope);
        }

        void pushScope() {
            this->scope = this->scope->addChild();
        }

        void popScope() {
            this->scope = this->scope->parent;
        }

        llvm::Module *encode(struct ast_module_t *m) {
            this->llvm_module = new llvm::Module("eokas", llvm_context);

            llvm_define_cfunc_puts(llvm_context, llvm_module);
            llvm_define_cfunc_printf(llvm_context, llvm_module);
            llvm_define_cfunc_sprintf(llvm_context, llvm_module);

            if (!this->encode_module(m)) {
                _DeletePointer(this->llvm_module);
                return nullptr;
            }
            return this->llvm_module;
        }

        llvm::Function *define_func_print() {
            llvm::StringRef name = "print";

            llvm::Type *ret = llvm::Type::getInt32Ty(llvm_context);

            std::vector<llvm::Type *> args = {
                    type_string
            };

            llvm::AttributeList attrs;

            auto funcType = llvm::FunctionType::get(ret, args, false);
            auto funcValue = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, name, llvm_module);
            funcValue->setCallingConv(llvm::CallingConv::C);
            funcValue->setAttributes(attrs);

            llvm::IRBuilder<> builder(llvm_context);

            llvm::BasicBlock *entry = llvm::BasicBlock::Create(llvm_context, "entry", funcValue);
            builder.SetInsertPoint(entry);

            llvm::Value *retval = llvm_invoke_code_print(entry, {funcValue->getArg(0)});
            builder.SetInsertPoint(entry);

            builder.CreateRet(retval);

            return funcValue;
        }

        bool encode_module(struct ast_module_t *node) {
            if (node == nullptr)
                return false;

            this->pushScope();

            this->scope->types["void"] = type_void;
            this->scope->types["i8"] = type_i8;
            this->scope->types["i16"] = type_i16;
            this->scope->types["i32"] = type_i32;
            this->scope->types["i64"] = type_i64;
            this->scope->types["u8"] = type_u8;
            this->scope->types["u16"] = type_u16;
            this->scope->types["u32"] = type_u32;
            this->scope->types["u64"] = type_u64;
            this->scope->types["f32"] = type_f32;
            this->scope->types["f64"] = type_f64;
            this->scope->types["bool"] = type_bool;
            this->scope->types["string"] = type_string;

            this->scope->symbols["print"] = define_func_print();

            llvm::FunctionType *funcType = llvm::FunctionType::get(type_i32, false);
            this->func = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, "main", llvm_module);

            llvm::BasicBlock *entry = llvm::BasicBlock::Create(llvm_context, "entry", this->func);
            llvm_builder.SetInsertPoint(entry);

            for (auto &stmt: node->stmts) {
                if (!this->encode_stmt(stmt))
                    return false;
            }

            llvm_builder.CreateRet(const_zero);

            this->popScope();

            return true;
        }

        llvm::Type *encode_type(struct ast_type_t *node) {
            if (node == nullptr)
                return nullptr;

            switch (node->category) {
                case ast_node_category_t::type_ref:
                    return this->encode_type_ref(dynamic_cast<ast_type_ref_t *>(node));
                case ast_node_category_t::type_array:
                    return this->encode_type_array(dynamic_cast<ast_type_array_t *>(node));
                default:
                    return nullptr;
            }

            return nullptr;
        }

        llvm::Type *encode_type_ref(struct ast_type_ref_t *node) {
            if (node == nullptr) {
                printf("type node is null. \n");
                return nullptr;
            }

            const String &name = node->name;
            llvm::Type *type = this->scope->getType(name, true);
            return type;
        }

        llvm::Type *encode_type_array(struct ast_type_array_t *node) {
            if (node == nullptr) {
                printf("type node is null. \n");
                return nullptr;
            }

            llvm::Type *elementType = this->encode_type(node->elementType);
            if (elementType == nullptr)
                return nullptr;

            llvm::Type *type = llvm::ArrayType::get(elementType, node->length);

            return type;
        }

        llvm::Value *encode_expr(struct ast_expr_t *node) {
            if (node == nullptr)
                return nullptr;

            switch (node->category) {
                case ast_node_category_t::expr_trinary:
                    return this->encode_expr_trinary(dynamic_cast<ast_expr_trinary_t *>(node));
                case ast_node_category_t::expr_binary_type:
                    return this->encode_expr_binary_type(dynamic_cast<ast_expr_binary_type_t *>(node));
                case ast_node_category_t::expr_binary_value:
                    return this->encode_expr_binary_value(dynamic_cast<ast_expr_binary_value_t *>(node));
                case ast_node_category_t::expr_unary:
                    return this->encode_expr_unary(dynamic_cast<ast_expr_unary_t *>(node));
                case ast_node_category_t::expr_int:
                    return this->encode_expr_int(dynamic_cast<ast_expr_int_t *>(node));
                case ast_node_category_t::expr_float:
                    return this->encode_expr_float(dynamic_cast<ast_expr_float_t *>(node));
                case ast_node_category_t::expr_bool:
                    return this->encode_expr_bool(dynamic_cast<ast_expr_bool_t *>(node));
                case ast_node_category_t::expr_string:
                    return this->encode_expr_string(dynamic_cast<ast_expr_string_t *>(node));
                case ast_node_category_t::expr_symbol_ref:
                    return this->encode_expr_symbol_ref(dynamic_cast<ast_expr_symbol_ref_t *>(node));
                case ast_node_category_t::expr_func_def:
                    return this->encode_expr_func_def(dynamic_cast<ast_expr_func_def_t *>(node));
                case ast_node_category_t::expr_func_ref:
                    return this->encode_expr_func_ref(dynamic_cast<ast_expr_func_ref_t *>(node));
                case ast_node_category_t::expr_array_def:
                    return this->encode_expr_array_def(dynamic_cast<ast_expr_array_def_t *>(node));
                case ast_node_category_t::expr_index_ref:
                    return this->encode_expr_index_ref(dynamic_cast<ast_expr_index_ref_t *>(node));
                case ast_node_category_t::expr_object_def:
                    return this->encode_expr_object_def(dynamic_cast<ast_expr_object_def_t *>(node));
                case ast_node_category_t::expr_object_ref:
                    return this->encode_expr_object_ref(dynamic_cast<ast_expr_object_ref_t *>(node));
                default:
                    return nullptr;
            }
            return nullptr;
        }

        llvm::Value *encode_expr_trinary(struct ast_expr_trinary_t *node) {
            if (node == nullptr)
                return nullptr;

            llvm::BasicBlock *trinary_begin = llvm::BasicBlock::Create(llvm_context, "trinary.begin", this->func);
            llvm::BasicBlock *trinary_true = llvm::BasicBlock::Create(llvm_context, "trinary.true", this->func);
            llvm::BasicBlock *trinary_false = llvm::BasicBlock::Create(llvm_context, "trinary.false", this->func);
            llvm::BasicBlock *trinary_end = llvm::BasicBlock::Create(llvm_context, "trinary.end", this->func);

            llvm_builder.SetInsertPoint(trinary_begin);

            // TODO: define a temp var

            llvm::Value *cond = this->encode_expr(node->cond);
            if (cond == nullptr)
                return nullptr;
            llvm_builder.CreateCondBr(cond, trinary_true, trinary_false);

            llvm_builder.SetInsertPoint(trinary_true);
            llvm::Value *trueV = this->encode_expr(node->branch_true);
            if (trueV == nullptr)
                return nullptr;
            // TODO: set temp var
            llvm_builder.CreateBr(trinary_end);

            llvm_builder.SetInsertPoint(trinary_false);
            llvm::Value *falseV = this->encode_expr(node->branch_false);
            if (falseV == nullptr)
                return nullptr;
            // TODO: set temp var
            llvm_builder.CreateBr(trinary_end);

            llvm_builder.SetInsertPoint(trinary_end);

            // TODO: return temp var

            return nullptr;
        }

        llvm::Value *encode_expr_binary_type(struct ast_expr_binary_type_t *node) {
            if(node == nullptr)
                return nullptr;

            auto lhs = this->encode_expr(node->left);
            auto rhs = this->encode_type(node->right);
            if (lhs == nullptr || rhs == nullptr)
                return nullptr;
            while (lhs->getType()->isPointerTy()) {
                lhs = llvm_builder.CreateLoad(lhs);
            }

            switch(node->op) {
                case ast_binary_oper_t::Is:
                {
                    if(rhs == lhs->getType())
                        return llvm_builder.getTrue();
                    else
                        return llvm_builder.getFalse();
                }
                break;
                case ast_binary_oper_t::As:
                {
                    if(lhs->getType()->isSingleValueType()) {
                        if(rhs == type_string) {
                            llvm::BasicBlock* block = llvm_builder.GetInsertBlock();
                            llvm::Value *value = llvm_invoke_code_as_string(block, {lhs});
                            llvm_builder.SetInsertPoint(block);
                            return value;
                        }
                        return nullptr;
                    }
                    else {
                        printf("complex cast is not supported.\n");
                        return nullptr;
                    }
                }
                break;
                default:
                    return nullptr;
            }

            return nullptr;
        }

        llvm::Value *encode_expr_binary_value(struct ast_expr_binary_value_t *node) {
            if (node == nullptr)
                return nullptr;

            auto lhs = this->encode_expr(node->left);
            auto rhs = this->encode_expr(node->right);
            if (lhs == nullptr || rhs == nullptr)
                return nullptr;
            if (lhs->getType()->isPointerTy()) {
                lhs = llvm_builder.CreateLoad(lhs);
            }
            if (rhs->getType()->isPointerTy()) {
                rhs = llvm_builder.CreateLoad(rhs);
            }

            switch (node->op) {
                case ast_binary_oper_t::Or:
                    return this->encode_expr_binary_or(lhs, rhs);
                case ast_binary_oper_t::And:
                    return this->encode_expr_binary_and(lhs, rhs);
                case ast_binary_oper_t::Equal:
                    return this->encode_expr_binary_eq(lhs, rhs);
                case ast_binary_oper_t::NEqual:
                    return this->encode_expr_binary_ne(lhs, rhs);
                case ast_binary_oper_t::LEqual:
                    return this->encode_expr_binary_le(lhs, rhs);
                case ast_binary_oper_t::GEqual:
                    return this->encode_expr_binary_ge(lhs, rhs);
                case ast_binary_oper_t::Less:
                    return this->encode_expr_binary_lt(lhs, rhs);
                case ast_binary_oper_t::Greater:
                    return this->encode_expr_binary_gt(lhs, rhs);
                case ast_binary_oper_t::Add:
                    return this->encode_expr_binary_add(lhs, rhs);
                case ast_binary_oper_t::Sub:
                    return this->encode_expr_binary_sub(lhs, rhs);
                case ast_binary_oper_t::Mul:
                    return this->encode_expr_binary_mul(lhs, rhs);
                case ast_binary_oper_t::Div:
                    return this->encode_expr_binary_div(lhs, rhs);
                case ast_binary_oper_t::Mod:
                    return this->encode_expr_binary_mod(lhs, rhs);
                case ast_binary_oper_t::BitAnd:
                    return this->encode_expr_binary_bitand(lhs, rhs);
                case ast_binary_oper_t::BitOr:
                    return this->encode_expr_binary_bitor(lhs, rhs);
                case ast_binary_oper_t::BitXor:
                    return this->encode_expr_binary_bitxor(lhs, rhs);
                case ast_binary_oper_t::ShiftL:
                    return llvm_builder.CreateShl(lhs, rhs);
                case ast_binary_oper_t::ShiftR:
                    return llvm_builder.CreateLShr(lhs, rhs);
                default:
                    return nullptr;
            }
        }

        llvm::Value *encode_expr_binary_or(llvm::Value *lhs, llvm::Value *rhs) {
            auto ltype = lhs->getType();
            auto rtype = rhs->getType();

            if ((!ltype->isIntegerTy(1)) || (!rtype->isIntegerTy(1))) {
                printf("LHS or RHS is not bool value. \n");
                return nullptr;
            }

            return llvm_builder.CreateOr(lhs, rhs);
        }

        llvm::Value *encode_expr_binary_and(llvm::Value *lhs, llvm::Value *rhs) {
            auto ltype = lhs->getType();
            auto rtype = rhs->getType();

            if ((!ltype->isIntegerTy(1)) || (!rtype->isIntegerTy(1))) {
                printf("LHS or RHS is not bool value. \n");
                return nullptr;
            }

            return llvm_builder.CreateAnd(lhs, rhs);
        }

        llvm::Value *encode_expr_binary_eq(llvm::Value *lhs, llvm::Value *rhs) {
            auto ltype = lhs->getType();
            auto rtype = rhs->getType();

            if (ltype->isIntegerTy() && rtype->isIntegerTy())
                return llvm_builder.CreateICmpEQ(lhs, rhs);

            if (ltype->isFloatingPointTy() && rtype->isFloatingPointTy())
                return llvm_builder.CreateFCmpOEQ(lhs, rhs);

            if (ltype->isPointerTy() && rtype->isPointerTy()) {
                return llvm_builder.CreateICmpEQ(
                        llvm_builder.CreatePtrToInt(lhs, llvm::Type::getInt64Ty(llvm_context)),
                        llvm_builder.CreatePtrToInt(rhs, llvm::Type::getInt64Ty(llvm_context)));
            }

            if (ltype->isIntegerTy() && rtype->isFloatingPointTy()) {
                return llvm_builder.CreateFCmpOEQ(
                        llvm_builder.CreateSIToFP(lhs, llvm::Type::getDoubleTy(llvm_context)),
                        rhs);
            }

            if (ltype->isFloatingPointTy() && rtype->isIntegerTy()) {
                return llvm_builder.CreateFCmpOEQ(
                        lhs,
                        llvm_builder.CreateSIToFP(rhs, llvm::Type::getDoubleTy(llvm_context)));
            }

            printf("Type of LHS or RHS is invalid.");
            return nullptr;
        }

        llvm::Value *encode_expr_binary_ne(llvm::Value *lhs, llvm::Value *rhs) {
            auto ltype = lhs->getType();
            auto rtype = rhs->getType();

            if (ltype->isIntegerTy() && rtype->isIntegerTy())
                return llvm_builder.CreateICmpNE(lhs, rhs);

            if (ltype->isFloatingPointTy() && rtype->isFloatingPointTy())
                return llvm_builder.CreateFCmpONE(lhs, rhs);

            if (ltype->isPointerTy() && rtype->isPointerTy()) {
                return llvm_builder.CreateICmpNE(
                        llvm_builder.CreatePtrToInt(lhs, llvm::Type::getInt64Ty(llvm_context)),
                        llvm_builder.CreatePtrToInt(rhs, llvm::Type::getInt64Ty(llvm_context)));
            }

            if (ltype->isIntegerTy() && rtype->isFloatingPointTy()) {
                return llvm_builder.CreateFCmpONE(
                        llvm_builder.CreateSIToFP(lhs, llvm::Type::getDoubleTy(llvm_context)),
                        rhs);
            }

            if (ltype->isFloatingPointTy() && rtype->isIntegerTy()) {
                return llvm_builder.CreateFCmpONE(
                        lhs,
                        llvm_builder.CreateSIToFP(rhs, llvm::Type::getDoubleTy(llvm_context)));
            }

            printf("Type of LHS or RHS is invalid.");
            return nullptr;
        }

        llvm::Value *encode_expr_binary_le(llvm::Value *lhs, llvm::Value *rhs) {
            auto ltype = lhs->getType();
            auto rtype = rhs->getType();

            if (ltype->isIntegerTy() && rtype->isIntegerTy())
                return llvm_builder.CreateICmpSLE(lhs, rhs);

            if (ltype->isFloatingPointTy() && rtype->isFloatingPointTy())
                return llvm_builder.CreateFCmpOLE(lhs, rhs);

            if (ltype->isPointerTy() && rtype->isPointerTy()) {
                return llvm_builder.CreateICmpULE(
                        llvm_builder.CreatePtrToInt(lhs, llvm::Type::getInt64Ty(llvm_context)),
                        llvm_builder.CreatePtrToInt(rhs, llvm::Type::getInt64Ty(llvm_context)));
            }

            if (ltype->isIntegerTy() && rtype->isFloatingPointTy()) {
                return llvm_builder.CreateFCmpOLE(
                        llvm_builder.CreateSIToFP(lhs, llvm::Type::getDoubleTy(llvm_context)),
                        rhs);
            }

            if (ltype->isFloatingPointTy() && rtype->isIntegerTy()) {
                return llvm_builder.CreateFCmpOLE(
                        lhs,
                        llvm_builder.CreateSIToFP(rhs, llvm::Type::getDoubleTy(llvm_context)));
            }

            printf("Type of LHS or RHS is invalid.");
            return nullptr;
        }

        llvm::Value *encode_expr_binary_ge(llvm::Value *lhs, llvm::Value *rhs) {
            auto ltype = lhs->getType();
            auto rtype = rhs->getType();

            if (ltype->isIntegerTy() && rtype->isIntegerTy())
                return llvm_builder.CreateICmpSGE(lhs, rhs);

            if (ltype->isFloatingPointTy() && rtype->isFloatingPointTy())
                return llvm_builder.CreateFCmpOGE(lhs, rhs);

            if (ltype->isPointerTy() && rtype->isPointerTy()) {
                return llvm_builder.CreateICmpUGE(
                        llvm_builder.CreatePtrToInt(lhs, llvm::Type::getInt64Ty(llvm_context)),
                        llvm_builder.CreatePtrToInt(rhs, llvm::Type::getInt64Ty(llvm_context)));
            }

            if (ltype->isIntegerTy() && rtype->isFloatingPointTy()) {
                return llvm_builder.CreateFCmpOGE(
                        llvm_builder.CreateSIToFP(lhs, llvm::Type::getDoubleTy(llvm_context)),
                        rhs);
            }

            if (ltype->isFloatingPointTy() && rtype->isIntegerTy()) {
                return llvm_builder.CreateFCmpOGE(
                        lhs,
                        llvm_builder.CreateSIToFP(rhs, llvm::Type::getDoubleTy(llvm_context)));
            }

            printf("Type of LHS or RHS is invalid.");
            return nullptr;
        }

        llvm::Value *encode_expr_binary_lt(llvm::Value *lhs, llvm::Value *rhs) {
            auto ltype = lhs->getType();
            auto rtype = rhs->getType();

            if (ltype->isIntegerTy() && rtype->isIntegerTy())
                return llvm_builder.CreateICmpSLT(lhs, rhs);

            if (ltype->isFloatingPointTy() && rtype->isFloatingPointTy())
                return llvm_builder.CreateFCmpOLT(lhs, rhs);

            if (ltype->isPointerTy() && rtype->isPointerTy()) {
                return llvm_builder.CreateICmpULT(
                        llvm_builder.CreatePtrToInt(lhs, llvm::Type::getInt64Ty(llvm_context)),
                        llvm_builder.CreatePtrToInt(rhs, llvm::Type::getInt64Ty(llvm_context)));
            }

            if (ltype->isIntegerTy() && rtype->isFloatingPointTy()) {
                return llvm_builder.CreateFCmpOLT(
                        llvm_builder.CreateSIToFP(lhs, llvm::Type::getDoubleTy(llvm_context)),
                        rhs);
            }

            if (ltype->isFloatingPointTy() && rtype->isIntegerTy()) {
                return llvm_builder.CreateFCmpOLT(
                        lhs,
                        llvm_builder.CreateSIToFP(rhs, llvm::Type::getDoubleTy(llvm_context)));
            }

            printf("Type of LHS or RHS is invalid.");
            return nullptr;
        }

        llvm::Value *encode_expr_binary_gt(llvm::Value *lhs, llvm::Value *rhs) {
            auto ltype = lhs->getType();
            auto rtype = rhs->getType();

            if (ltype->isIntegerTy() && rtype->isIntegerTy())
                return llvm_builder.CreateICmpSGT(lhs, rhs);

            if (ltype->isFloatingPointTy() && rtype->isFloatingPointTy())
                return llvm_builder.CreateFCmpOGT(lhs, rhs);

            if (ltype->isPointerTy() && rtype->isPointerTy()) {
                return llvm_builder.CreateICmpUGT(
                        llvm_builder.CreatePtrToInt(lhs, llvm::Type::getInt64Ty(llvm_context)),
                        llvm_builder.CreatePtrToInt(rhs, llvm::Type::getInt64Ty(llvm_context)));
            }

            if (ltype->isIntegerTy() && rtype->isFloatingPointTy()) {
                return llvm_builder.CreateFCmpOGT(
                        llvm_builder.CreateSIToFP(lhs, llvm::Type::getDoubleTy(llvm_context)),
                        rhs);
            }

            if (ltype->isFloatingPointTy() && rtype->isIntegerTy()) {
                return llvm_builder.CreateFCmpOGT(
                        lhs,
                        llvm_builder.CreateSIToFP(rhs, llvm::Type::getDoubleTy(llvm_context)));
            }

            printf("Type of LHS or RHS is invalid.");
            return nullptr;
        }

        llvm::Value *encode_expr_binary_add(llvm::Value *lhs, llvm::Value *rhs) {
            auto ltype = lhs->getType();
            auto rtype = rhs->getType();

            if (ltype->isIntegerTy() && rtype->isIntegerTy())
                return llvm_builder.CreateAdd(lhs, rhs);

            if (ltype->isFloatingPointTy() && rtype->isFloatingPointTy())
                return llvm_builder.CreateFAdd(lhs, rhs);

            if (ltype->isIntegerTy() && rtype->isFloatingPointTy()) {
                return llvm_builder.CreateFAdd(
                        llvm_builder.CreateSIToFP(lhs, llvm::Type::getDoubleTy(llvm_context)),
                        rhs);
            }

            if (ltype->isFloatingPointTy() && rtype->isIntegerTy()) {
                return llvm_builder.CreateFAdd(
                        lhs,
                        llvm_builder.CreateSIToFP(rhs, llvm::Type::getDoubleTy(llvm_context)));
            }

            printf("Type of LHS or RHS is invalid.");
            return nullptr;
        }

        llvm::Value *encode_expr_binary_sub(llvm::Value *lhs, llvm::Value *rhs) {
            auto ltype = lhs->getType();
            auto rtype = rhs->getType();

            if (ltype->isIntegerTy() && rtype->isIntegerTy())
                return llvm_builder.CreateSub(lhs, rhs);

            if (ltype->isFloatingPointTy() && rtype->isFloatingPointTy())
                return llvm_builder.CreateFSub(lhs, rhs);

            if (ltype->isIntegerTy() && rtype->isFloatingPointTy()) {
                return llvm_builder.CreateFSub(
                        llvm_builder.CreateSIToFP(lhs, llvm::Type::getDoubleTy(llvm_context)),
                        rhs);
            }

            if (ltype->isFloatingPointTy() && rtype->isIntegerTy()) {
                return llvm_builder.CreateFSub(
                        lhs,
                        llvm_builder.CreateSIToFP(rhs, llvm::Type::getDoubleTy(llvm_context)));
            }

            printf("Type of LHS or RHS is invalid.");
            return nullptr;
        }

        llvm::Value *encode_expr_binary_mul(llvm::Value *lhs, llvm::Value *rhs) {
            auto ltype = lhs->getType();
            auto rtype = rhs->getType();

            if (ltype->isIntegerTy() && rtype->isIntegerTy())
                return llvm_builder.CreateMul(lhs, rhs);

            if (ltype->isFloatingPointTy() && rtype->isFloatingPointTy())
                return llvm_builder.CreateFMul(lhs, rhs);

            if (ltype->isIntegerTy() && rtype->isFloatingPointTy()) {
                return llvm_builder.CreateFMul(
                        llvm_builder.CreateSIToFP(lhs, llvm::Type::getDoubleTy(llvm_context)),
                        rhs);
            }

            if (ltype->isFloatingPointTy() && rtype->isIntegerTy()) {
                return llvm_builder.CreateFMul(
                        lhs,
                        llvm_builder.CreateSIToFP(rhs, llvm::Type::getDoubleTy(llvm_context)));
            }

            printf("Type of LHS or RHS is invalid.");
            return nullptr;
        }

        llvm::Value *encode_expr_binary_div(llvm::Value *lhs, llvm::Value *rhs) {
            auto ltype = lhs->getType();
            auto rtype = rhs->getType();

            if (ltype->isIntegerTy() && rtype->isIntegerTy())
                return llvm_builder.CreateSDiv(lhs, rhs);

            if (ltype->isFloatingPointTy() && rtype->isFloatingPointTy())
                return llvm_builder.CreateFMul(lhs, rhs);

            if (ltype->isIntegerTy() && rtype->isFloatingPointTy()) {
                return llvm_builder.CreateFDiv(
                        llvm_builder.CreateSIToFP(lhs, llvm::Type::getDoubleTy(llvm_context)),
                        rhs);
            }

            if (ltype->isFloatingPointTy() && rtype->isIntegerTy()) {
                return llvm_builder.CreateFDiv(
                        lhs,
                        llvm_builder.CreateSIToFP(rhs, llvm::Type::getDoubleTy(llvm_context)));
            }

            printf("Type of LHS or RHS is invalid.");
            return nullptr;
        }

        llvm::Value *encode_expr_binary_mod(llvm::Value *lhs, llvm::Value *rhs) {
            auto ltype = lhs->getType();
            auto rtype = rhs->getType();

            if (ltype->isIntegerTy() && rtype->isIntegerTy())
                return llvm_builder.CreateSRem(lhs, rhs);

            if (ltype->isFloatingPointTy() && rtype->isFloatingPointTy())
                return llvm_builder.CreateFRem(lhs, rhs);

            if (ltype->isIntegerTy() && rtype->isFloatingPointTy()) {
                return llvm_builder.CreateFRem(
                        llvm_builder.CreateSIToFP(lhs, llvm::Type::getDoubleTy(llvm_context)),
                        rhs);
            }

            if (ltype->isFloatingPointTy() && rtype->isIntegerTy()) {
                return llvm_builder.CreateFRem(
                        lhs,
                        llvm_builder.CreateSIToFP(rhs, llvm::Type::getDoubleTy(llvm_context)));
            }

            printf("Type of LHS or RHS is invalid.");
            return nullptr;
        }

        llvm::Value *encode_expr_binary_bitand(llvm::Value *lhs, llvm::Value *rhs) {
            auto ltype = lhs->getType();
            auto rtype = rhs->getType();

            if (ltype->isIntegerTy() && rtype->isIntegerTy())
                return llvm_builder.CreateAnd(lhs, rhs);

            printf("Type of LHS or RHS is invalid.");
            return nullptr;
        }

        llvm::Value *encode_expr_binary_bitor(llvm::Value *lhs, llvm::Value *rhs) {
            auto ltype = lhs->getType();
            auto rtype = rhs->getType();

            if (ltype->isIntegerTy() && rtype->isIntegerTy())
                return llvm_builder.CreateOr(lhs, rhs);

            printf("Type of LHS or RHS is invalid.");
            return nullptr;
        }

        llvm::Value *encode_expr_binary_bitxor(llvm::Value *lhs, llvm::Value *rhs) {
            auto ltype = lhs->getType();
            auto rtype = rhs->getType();

            if (ltype->isIntegerTy() && rtype->isIntegerTy())
                return llvm_builder.CreateXor(lhs, rhs);

            printf("Type of LHS or RHS is invalid.");
            return nullptr;
        }

        llvm::Value *encode_expr_binary_bitshl(llvm::Value *lhs, llvm::Value *rhs) {
            auto ltype = lhs->getType();
            auto rtype = rhs->getType();

            if (ltype->isIntegerTy() && rtype->isIntegerTy())
                return llvm_builder.CreateShl(lhs, rhs);

            printf("Type of LHS or RHS is invalid.");
            return nullptr;
        }

        llvm::Value *encode_expr_binary_bitshr(llvm::Value *lhs, llvm::Value *rhs) {
            auto ltype = lhs->getType();
            auto rtype = rhs->getType();

            if (ltype->isIntegerTy() && rtype->isIntegerTy()) {
                // 逻辑右移：在左边补 0
                // 算术右移：在左边补 符号位
                // 我们采用逻辑右移
                return llvm_builder.CreateLShr(lhs, rhs);
            }

            printf("Type of LHS or RHS is invalid.");
            return nullptr;
        }

        llvm::Value *encode_expr_unary(struct ast_expr_unary_t *node) {
            if (node == nullptr)
                return nullptr;

            auto rhs = this->encode_expr(node->right);
            if (rhs == nullptr)
                return nullptr;

            if (rhs->getType()->isPointerTy()) {
                rhs = llvm_builder.CreateLoad(rhs);
            }

            switch (node->op) {
                case ast_unary_oper_t::Pos:
                    return rhs;
                case ast_unary_oper_t::Neg:
                    return this->encode_expr_unary_neg(rhs);
                case ast_unary_oper_t::Not:
                    return this->encode_expr_unary_not(rhs);
                case ast_unary_oper_t::Flip:
                    return this->encode_expr_unary_flip(rhs);
                default:
                    return nullptr;
            }
        }

        llvm::Value *encode_expr_unary_neg(llvm::Value *rhs) {
            auto rtype = rhs->getType();

            if (rtype->isIntegerTy())
                return llvm_builder.CreateNeg(rhs);

            if (rtype->isFloatingPointTy())
                return llvm_builder.CreateFNeg(rhs);

            printf("Type of RHS is invalid.");
            return nullptr;
        }

        llvm::Value *encode_expr_unary_not(llvm::Value *rhs) {
            auto rtype = rhs->getType();

            if (rtype->isIntegerTy() && rtype->getIntegerBitWidth() == 1)
                return llvm_builder.CreateNot(rhs);

            printf("Type of RHS is invalid.");
            return nullptr;
        }

        llvm::Value *encode_expr_unary_flip(llvm::Value *rhs) {
            auto rtype = rhs->getType();

            if (rtype->isIntegerTy()) {
                return llvm_builder.CreateXor(
                        rhs,
                        llvm::ConstantInt::get(rtype, llvm::APInt(rtype->getIntegerBitWidth(), 0xFFFFFFFF)));
            }
            printf("Type of RHS is invalid.");
            return nullptr;
        }

        llvm::Value *encode_expr_int(struct ast_expr_int_t *node) {
            if (node == nullptr)
                return nullptr;

            u64_t vals = *((u64_t *) &(node->value));
            u32_t bits = vals > 0xFFFFFFFF ? 64 : 32;

            return llvm::ConstantInt::get(llvm_context, llvm::APInt(bits, node->value));
        }

        llvm::Value *encode_expr_float(struct ast_expr_float_t *node) {
            if (node == nullptr)
                return nullptr;

            return llvm::ConstantFP::get(llvm_context, llvm::APFloat(node->value));
        }

        llvm::Value *encode_expr_bool(struct ast_expr_bool_t *node) {
            if (node == nullptr)
                return nullptr;
            return llvm::ConstantInt::getBool(llvm_context, node->value);
        }

        llvm::Value *encode_expr_string(struct ast_expr_string_t *node) {
            if (node == nullptr)
                return nullptr;
            return llvm_builder.CreateGlobalString(node->value.cstr());
        }

        llvm::Value *encode_expr_symbol_ref(struct ast_expr_symbol_ref_t *node) {
            if (node == nullptr)
                return nullptr;

            auto symbol = this->scope->getSymbol(node->name, true);
            if (symbol == nullptr) {
                printf("symbol '%s' is undefined.\n", node->name.cstr());
                return nullptr;
            }

            return symbol;
        }

        llvm::Value *encode_expr_func_def(struct ast_expr_func_def_t *node) {
            if (node == nullptr)
                return nullptr;

            llvm::Type *retType = this->encode_type(node->type);
            if (retType == nullptr)
                return nullptr;

            std::vector<llvm::Type *> argTypes;
            for (auto arg: node->args) {
                llvm::Type *argType = this->encode_type(arg.type);
                if (argType == nullptr)
                    return nullptr;
                argTypes.push_back(argType);
            }

            llvm::FunctionType *procType = llvm::FunctionType::get(retType, argTypes, false);
            llvm::Function *func = llvm::Function::Create(procType, llvm::Function::ExternalLinkage, "", llvm_module);
            u32_t index = 0;
            for (auto &arg: func->args()) {
                const char *name = node->args[index++].name.cstr();
                arg.setName(name);
            }

            auto oldFunc = this->func;
            auto oldIB = llvm_builder.GetInsertBlock();
            this->func = func;
            this->pushScope();
            {
                llvm::BasicBlock *entry = llvm::BasicBlock::Create(llvm_context, "entry", func);
                llvm_builder.SetInsertPoint(entry);

                for (auto &arg: func->args()) {
                    const char *name = arg.getName().data();
                    llvm::Value *ptr = llvm_builder.CreateAlloca(arg.getType());
                    llvm_builder.CreateStore(&arg, ptr);
                    auto pair = std::make_pair(String(name), ptr);
                    this->scope->symbols.insert(pair);
                }

                for (auto &stmt: node->body) {
                    if (!this->encode_stmt(stmt))
                        return nullptr;
                }
            }

            this->popScope();
            this->func = oldFunc;
            llvm_builder.SetInsertPoint(oldIB);

            return func;
        }

        llvm::Value *encode_expr_func_ref(struct ast_expr_func_ref_t *node) {
            if (node == nullptr)
                return nullptr;

            llvm::Value *funcValue = this->encode_expr(node->func);
            if(funcValue == nullptr) {
                printf("Function is undefined.\n");
                return nullptr;
            }

            llvm::Function *func = llvm::cast<llvm::Function>(funcValue);
            if (func == nullptr)
                return nullptr;
            if (func->arg_size() != node->args.size())
                return nullptr;

            std::vector<llvm::Value *> params;
            for (auto &arg: node->args) {
                llvm::Value *param = this->encode_expr(arg);
                if (param == nullptr)
                    return nullptr;
                params.push_back(param);
            }

            return llvm_builder.CreateCall(func, params);
        }

        llvm::Value *encode_expr_array_def(struct ast_expr_array_def_t *node) {
            if (node == nullptr)
                return nullptr;

            printf("1 \n");
            std::vector<llvm::Constant *> arrayItems;
            llvm::Type *itemType = nullptr;
            for (auto item: node->items) {
                printf("2 \n");
                auto val = this->encode_expr(item);
                if (val == nullptr)
                    return nullptr;
                auto constant = llvm::cast<llvm::Constant>(val);
                if (constant == nullptr)
                    return nullptr;

                auto thisItemType = constant->getType();
                if (itemType == nullptr) {
                    itemType = thisItemType;
                }

                arrayItems.push_back(constant);
            }

            if (arrayItems.empty() || itemType == nullptr)
                itemType = llvm::Type::getInt32Ty(llvm_context);

            printf("3 \n");
            auto arrayType = llvm::ArrayType::get(itemType, 0);
            auto arrayValue = llvm::ConstantArray::get(arrayType, arrayItems);

            printf("4 \n");
            return arrayValue;
        }

        llvm::Value *encode_expr_index_ref(struct ast_expr_index_ref_t *node) {
            if (node == nullptr)
                return nullptr;

            auto obj = this->encode_expr(node->obj);
            auto key = this->encode_expr(node->key);
            if (obj == nullptr || key == nullptr)
                return nullptr;

            auto objType = obj->getType();
            auto keyType = key->getType();

            if (objType->isArrayTy() && keyType->isIntegerTy()) {
                llvm::Value *value = llvm_builder.CreateGEP(obj, key);
                return value;
            }

            return nullptr;
        }

        llvm::Value *encode_expr_object_def(struct ast_expr_object_def_t *node) {
            if (node == nullptr)
                return nullptr;


            llvm::PointerType *structRefType = llvm::cast<llvm::PointerType>(this->encode_type(node->type));
            if (structRefType == nullptr)
                return nullptr;
            llvm::StructType *structType = llvm::cast<llvm::StructType>(structRefType->getElementType());
            if (structType == nullptr)
                return nullptr;
            auto iter = this->structs.find(structType);
            if (iter == this->structs.end())
                return nullptr;
            ast_stmt_struct_def_t *structDef = iter->second;

            for (auto &mem: node->members) {
                if (structDef->members.find(mem.first) == structDef->members.end()) {
                    printf("object member '%s' is not defined in struct.", mem.first.cstr());
                    return nullptr;
                }
            }

            llvm::Value *objectValue = llvm_builder.CreateAlloca(structType);

            u32_t index = -1;
            for (auto &mem: structDef->members) {
                index += 1;
                auto memNode = node->members.find(mem.first);
                llvm::Value *memValue = nullptr;
                if (memNode != node->members.end()) {
                    memValue = this->encode_expr(memNode->second);
                } else {
                    memValue = llvm::ConstantInt::get(type_i32, llvm::APInt(32, 0));
                }

                llvm::Value *memPtr = llvm_builder.CreateStructGEP(structType, objectValue, index, mem.first.cstr());
                llvm_builder.CreateStore(memValue, memPtr);
            }

            return objectValue;
        }

        llvm::Value *encode_expr_object_ref(struct ast_expr_object_ref_t *node) {
            printf("object ref 01\n");
            if (node == nullptr)
                return nullptr;

            auto obj = this->encode_expr(node->obj);
            auto key = llvm_builder.CreateGlobalString(node->key.cstr());
            if (obj == nullptr || key == nullptr)
                return nullptr;
            printf("object ref 02\n");
            auto objType = obj->getType();
            while (objType->isPointerTy()) {
                obj = llvm_builder.CreateLoad(obj);
                objType = obj->getType();
            }
            printf("object ref 03\n");
            if (!objType->isStructTy())
                return nullptr;
            printf("object ref 04\n");
            auto structIter = this->structs.find(objType);
            if (structIter == this->structs.end())
                return nullptr;
            printf("object ref 05\n");
            auto structDef = structIter->second;

            u32_t index = -1;
            for (auto &mem: structDef->members) {
                index += 1;
                if (mem.second->name == node->key)
                    break;
            }
            printf("object ref 06， index=%d\n", index);
            if (index < 0)
                return nullptr;

            llvm::Value *value = llvm_builder.CreateStructGEP(obj->getType(), obj, index);
            printf("object ref 07\n");
            value->getType()->print(llvm::errs());
            return value;
        }

        bool encode_stmt(struct ast_stmt_t *node) {
            if (node == nullptr)
                return false;

            switch (node->category) {
                case ast_node_category_t::stmt_schema_def:
                    return this->encode_stmt_schema_def(dynamic_cast<ast_stmt_schema_def_t *>(node));
                case ast_node_category_t::stmt_struct_def:
                    return this->encode_stmt_struct_def(dynamic_cast<ast_stmt_struct_def_t *>(node));
                case ast_node_category_t::stmt_proc_def:
                    return this->encode_stmt_proc_def(dynamic_cast<ast_stmt_proc_def_t *>(node));
                case ast_node_category_t::stmt_symbol_def:
                    return this->encode_stmt_symbol_def(dynamic_cast<ast_stmt_symbol_def_t *>(node));
                case ast_node_category_t::stmt_break:
                    return this->encode_stmt_break(dynamic_cast<ast_stmt_break_t *>(node));
                case ast_node_category_t::stmt_continue:
                    return this->encode_stmt_continue(dynamic_cast<ast_stmt_continue_t *>(node));
                case ast_node_category_t::stmt_return:
                    return this->encode_stmt_return(dynamic_cast<ast_stmt_return_t *>(node));
                case ast_node_category_t::stmt_if:
                    return this->encode_stmt_if(dynamic_cast<ast_stmt_if_t *>(node));
                case ast_node_category_t::stmt_while:
                    return this->encode_stmt_while(dynamic_cast<ast_stmt_while_t *>(node));
                case ast_node_category_t::stmt_for:
                    return this->encode_stmt_for(dynamic_cast<ast_stmt_for_t *>(node));
                case ast_node_category_t::stmt_block:
                    return this->encode_stmt_block(dynamic_cast<ast_stmt_block_t *>(node));
                case ast_node_category_t::stmt_assign:
                    return this->encode_stmt_assign(dynamic_cast<ast_stmt_assign_t *>(node));
                case ast_node_category_t::stmt_call:
                    return this->encode_stmt_call(dynamic_cast<ast_stmt_call_t *>(node));
                default:
                    return false;
            }

            return false;
        }

        bool encode_stmt_schema_def(struct ast_stmt_schema_def_t *node) {
            if (node == nullptr)
                return false;

            const String &name = node->name;

            std::vector<llvm::Constant *> memberNames;
            std::vector<llvm::Type *> memberTypes;
            auto tokenNameType = llvm::ArrayType::get(llvm::Type::getInt8Ty(llvm_context), 256);
            auto metaType = llvm::ArrayType::get(tokenNameType, node->members.size() + 1);
            memberNames.push_back(llvm::ConstantDataArray::getString(llvm_context, "$_meta"));
            memberTypes.push_back(metaType);
            for (auto node: node->members) {
                auto mem = node.second;
                const auto &memName = mem->name.cstr();
                const auto &memType = this->encode_type(mem->type);
                if (memType == nullptr)
                    return false;
                memberNames.push_back(llvm::ConstantDataArray::getString(llvm_context, memName));
                memberTypes.push_back(memType);
            }

            auto structType = llvm::StructType::get(llvm_context);
            structType->setBody(memberTypes);

            auto metaData = llvm::ConstantArray::get(metaType, memberNames);
            auto metaSymbol = llvm_builder.CreateAlloca(metaType);
            llvm_builder.CreateStore(metaData, metaSymbol);
            this->scope->symbols.insert(std::make_pair(String::format("schema.%s.meta", name.cstr()), metaSymbol));
            this->scope->types.insert(std::make_pair(name, structType));

            return true;
        }

        bool encode_stmt_struct_def(struct ast_stmt_struct_def_t *node) {
            if (node == nullptr)
                return false;

            const String &name = node->name;

            std::vector<llvm::Constant *> memberNames;
            std::vector<llvm::Type *> memberTypes;
            for (auto node: node->members) {
                auto mem = node.second;
                const auto &memName = mem->name.cstr();
                const auto &memType = this->encode_type(mem->type);
                if (memType == nullptr)
                    return false;

                memberNames.push_back(llvm_builder.CreateGlobalString(memName));
                memberTypes.push_back(memType);
            }

            String structName = String::format("struct.%s", name.cstr());
            auto structType = llvm::StructType::create(llvm_context);
            structType->setName(structName.cstr());
            structType->setBody(memberTypes);

            llvm::Type *structRefType = llvm::PointerType::get(structType, 0);

            this->scope->types.insert(std::make_pair(name, structRefType));
            this->structs.insert(std::make_pair(structType, node));

            return true;
        }

        bool encode_stmt_proc_def(struct ast_stmt_proc_def_t *node) {
            if (node == nullptr)
                return false;

            if (this->scope->getType(node->name, false) != nullptr)
                return false;

            llvm::Type *retType = this->encode_type(node->type);

            std::vector<llvm::Type *> argTypes;
            for (auto arg: node->args) {
                llvm::Type *argType = this->encode_type(arg.second);
                if (argType == nullptr)
                    return false;
                argTypes.push_back(argType);
            }

            llvm::FunctionType *procType = llvm::FunctionType::get(retType, argTypes, false);

            this->scope->types[node->name] = procType;

            return true;
        }

        bool encode_stmt_symbol_def(struct ast_stmt_symbol_def_t *node) {
            if (node == nullptr)
                return false;

            if (this->scope->getSymbol(node->name, false) != nullptr)
                return false;

            auto type = this->encode_type(node->type);
            auto value = this->encode_expr(node->value);
            if (value == nullptr)
                return false;

            auto vtype = value->getType();
            while (vtype->isPointerTy()) {
                value = llvm_builder.CreateLoad(value);
                vtype = vtype->getPointerElementType();
            }
            if (type != nullptr) {
                if (type->getTypeID() != vtype->getTypeID() && !vtype->canLosslesslyBitCastTo(type)) {
                    printf("type '%d' can not cast to type '%d'. \n", vtype->getTypeID(), type->getTypeID());
                    return false;
                }
            } else {
                type = vtype;
            }

            // TODO: 校验类型合法性, 值类型是否遵循标记类型
            // 不同的类型，需要调用不同的store命令

            auto symbol = llvm_builder.CreateAlloca(type, nullptr, node->name.cstr());
            llvm_builder.CreateStore(value, symbol);
            scope->symbols.insert(std::make_pair(node->name, symbol));

            return true;
        }

        bool encode_stmt_break(struct ast_stmt_break_t *node) {
            if (node == nullptr)
                return false;

            if (this->breakPoint == nullptr)
                return false;

            llvm_builder.CreateBr(this->breakPoint);

            return true;
        }

        bool encode_stmt_continue(struct ast_stmt_continue_t *node) {
            if (node == nullptr)
                return false;

            if (this->continuePoint == nullptr)
                return false;

            llvm_builder.CreateBr(this->continuePoint);

            return true;
        }

        bool encode_stmt_return(struct ast_stmt_return_t *node) {
            if (node == nullptr)
                return false;

            if (node->value == nullptr) {
                llvm_builder.CreateRetVoid();
                return true;
            }

            llvm::Value *value = this->encode_expr(node->value);
            if (value == nullptr) {
                printf("invalid ret value.\n");
                return false;
            }

            llvm_builder.CreateRet(value);

            return true;
        }

        bool encode_stmt_if(struct ast_stmt_if_t *node) {
            if (node == nullptr)
                return false;

            llvm::BasicBlock *if_begin = llvm::BasicBlock::Create(llvm_context, "if.begin", this->func);
            llvm::BasicBlock *if_true = llvm::BasicBlock::Create(llvm_context, "if.true", this->func);
            llvm::BasicBlock *if_false = llvm::BasicBlock::Create(llvm_context, "if.false", this->func);
            llvm::BasicBlock *if_end = llvm::BasicBlock::Create(llvm_context, "if.end", this->func);

            llvm_builder.CreateBr(if_begin);
            llvm_builder.SetInsertPoint(if_begin);
            llvm::Value *cond = this->encode_expr(node->cond);
            if (cond == nullptr)
                return false;
            while(cond->getType()->isPointerTy()) {
                cond = llvm_builder.CreateLoad(cond);
            }
            if (!cond->getType()->isIntegerTy(1)) {
                printf("if.cond need a bool value.\n");
                return false;
            }
            llvm_builder.CreateCondBr(cond, if_true, if_false);

            llvm_builder.SetInsertPoint(if_true);
            if (!this->encode_stmt(node->branch_true))
                return false;
            llvm_builder.CreateBr(if_end);

            if (node->branch_false != nullptr) {
                llvm_builder.SetInsertPoint(if_false);
                if (!this->encode_stmt(node->branch_false))
                    return false;
                llvm_builder.CreateBr(if_end);
            }

            llvm_builder.SetInsertPoint(if_end);

            return true;
        }

        bool encode_stmt_while(struct ast_stmt_while_t *node) {
            if (node == nullptr)
                return false;

            this->pushScope();

            llvm::BasicBlock *while_begin = llvm::BasicBlock::Create(llvm_context, "while.begin", this->func);
            llvm::BasicBlock *while_body = llvm::BasicBlock::Create(llvm_context, "while.body", this->func);
            llvm::BasicBlock *while_end = llvm::BasicBlock::Create(llvm_context, "while.end", this->func);

            auto oldContinuePoint = this->continuePoint;
            auto oldBreakPoint = this->breakPoint;

            this->continuePoint = while_begin;
            this->breakPoint = while_end;

            llvm_builder.CreateBr(while_begin);
            llvm_builder.SetInsertPoint(while_begin);
            llvm::Value *cond = this->encode_expr(node->cond);
            if (cond == nullptr)
                return false;
            if (!cond->getType()->isIntegerTy(1)) {
                printf("while.cond need a bool value.\n");
                return false;
            }
            llvm_builder.CreateCondBr(cond, while_body, while_end);

            llvm_builder.SetInsertPoint(while_body);
            if (!this->encode_stmt(node->body))
                return false;
            llvm_builder.CreateBr(while_begin);

            llvm_builder.SetInsertPoint(while_end);

            this->continuePoint = oldContinuePoint;
            this->breakPoint = oldBreakPoint;

            this->popScope();

            return true;
        }

        bool encode_stmt_for(struct ast_stmt_for_t *node) {
            if (node == nullptr)
                return false;

            this->pushScope();

            llvm::BasicBlock *for_init = llvm::BasicBlock::Create(llvm_context, "for.init", this->func);
            llvm::BasicBlock *for_test = llvm::BasicBlock::Create(llvm_context, "for.test", this->func);
            llvm::BasicBlock *for_step = llvm::BasicBlock::Create(llvm_context, "for.step", this->func);
            llvm::BasicBlock *for_body = llvm::BasicBlock::Create(llvm_context, "for.body", this->func);
            llvm::BasicBlock *for_end = llvm::BasicBlock::Create(llvm_context, "for.end", this->func);

            auto oldContinuePoint = this->continuePoint;
            auto oldBreakPoint = this->breakPoint;
            this->continuePoint = for_step;
            this->breakPoint = for_end;

            llvm_builder.CreateBr(for_init);
            llvm_builder.SetInsertPoint(for_init);
            if (!this->encode_stmt(node->init))
                return false;
            llvm_builder.CreateBr(for_test);

            llvm_builder.SetInsertPoint(for_test);
            llvm::Value *cond = this->encode_expr(node->cond);
            if (cond == nullptr)
                return false;
            if (!cond->getType()->isIntegerTy(1)) {
                printf("for.cond need a bool value.\n");
                return false;
            }
            llvm_builder.CreateCondBr(cond, for_body, for_end);

            llvm_builder.SetInsertPoint(for_body);
            if (!this->encode_stmt(node->body))
                return false;
            llvm_builder.CreateBr(for_step);

            llvm_builder.SetInsertPoint(for_step);
            if (!this->encode_stmt(node->step))
                return false;
            llvm_builder.CreateBr(for_test);

            llvm_builder.SetInsertPoint(for_end);

            this->continuePoint = oldContinuePoint;
            this->breakPoint = oldBreakPoint;

            this->popScope();

            return true;
        }

        bool encode_stmt_block(struct ast_stmt_block_t *node) {
            if (node == nullptr)
                return false;

            this->pushScope();

            for (auto &stmt: node->stmts) {
                if (!this->encode_stmt(stmt))
                    return false;
            }

            this->popScope();

            return true;
        }

        bool encode_stmt_assign(struct ast_stmt_assign_t *node) {
            if (node == nullptr)
                return false;

            llvm::Value *left = this->encode_expr(node->left);
            llvm::Value *right = this->encode_expr(node->right);

            llvm_builder.CreateStore(right, left);

            return true;
        }

        bool encode_stmt_call(struct ast_stmt_call_t *node) {
            if (node == nullptr)
                return false;

            if (!this->encode_expr_func_ref(node->expr))
                return false;

            return true;
        }
    };

    llvm::Module *llvm_encode(llvm::LLVMContext &context, ast_module_t *module) {
        llvm_coder_t coder(context);
        llvm::Module *llvm_module = coder.encode(module);
        return llvm_module;
    }

    llvm::Module *llvm_encode_test(llvm::LLVMContext &context) {
        llvm::Module *module = new llvm::Module("eokas-test", context);
        llvm::IRBuilder<> builder(context);

        llvm::Function *puts = llvm_define_cfunc_puts(context, module);
        llvm::Function *printf = llvm_define_cfunc_printf(context, module);
        llvm::Function *sprintf = llvm_define_cfunc_sprintf(context, module);
        llvm::Function *malloc = llvm_define_cfunc_malloc(context, module);
        llvm::Function *free = llvm_define_cfunc_free(context, module);

        llvm::Type *type_i32 = llvm::Type::getInt32Ty(context);
        llvm::Type *type_f32 = llvm::Type::getFloatTy(context);
        llvm::Type *type_f64 = llvm::Type::getDoubleTy(context);
        llvm::Type *type_str = llvm::Type::getInt8PtrTy(context);

        llvm::FunctionType *funcType = llvm::FunctionType::get(type_i32, false);
        auto func = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, "main", module);

        llvm::BasicBlock *entry = llvm::BasicBlock::Create(context, "entry", func);
        builder.SetInsertPoint(entry);

        llvm::Value *zero = llvm::ConstantInt::get(type_i32, llvm::APInt(32, 0));

        llvm::Value *i = llvm::ConstantInt::get(type_i32, llvm::APInt(32, 100));
        llvm::Value *pi = llvm::ConstantFP::get(type_f64, llvm::APFloat(3.141592653));

        llvm::Value *ret = llvm_invoke_code_print(entry, {pi});
        builder.SetInsertPoint(entry);

        builder.CreateRet(zero);

        return module;
    }

_EndNamespace(eokas)
