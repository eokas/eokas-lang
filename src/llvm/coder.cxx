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

namespace eokas
{
	struct llvm_coder_t : llvm_module_t
	{
		llvm_function_t* func;
		llvm::BasicBlock* continuePoint;
		llvm::BasicBlock* breakPoint;
		
		explicit llvm_coder_t(llvm::LLVMContext& context, const String& name)
			: llvm_module_t(context, name)
		{
			this->func = nullptr;
			this->continuePoint = nullptr;
			this->breakPoint = nullptr;
		}
		
		virtual ~llvm_coder_t()
		{
		}
		
		void pushScope(llvm_function_t* f = nullptr)
		{
			this->scope = this->scope->add_child(f);
		}
		
		void popScope()
		{
			this->scope = this->scope->parent;
		}
		
		bool encode_module(ast_node_module_t* node)
		{
			if(node == nullptr)
				return false;
			
			auto* retT = type_i32;
			std::vector<llvm::Type*> argsT;
			
			auto* func = new llvm_function_t(*this, "main", retT, argsT, false);
			this->add_value("$main", func);
			
			this->func = func;
			
			llvm::IRBuilder<>& IR = func->IR;
			
			this->pushScope(func);
			{
				auto entry = func->add_basic_block("entry");
				IR.SetInsertPoint(entry);
				
				for (auto& stmt: node->entry->body)
				{
					if(!this->encode_stmt(stmt))
						return false;
				}
				
				func->add_tail_ret();
			}
			this->popScope();
			
			return true;
		}
		
		llvm_type_t* encode_type(ast_node_type_t* node)
		{
			if(node == nullptr)
			{
				printf("Type node is null. \n");
				return nullptr;
			}
			
			const String& name = node->name;
			
			// TODO: implements array by Generic-Type
			if(name == "Array")
			{
				auto* elementT = this->encode_type(node->args[0]);
				if(elementT == nullptr)
				{
					printf("Invalid array element type. \n");
					return nullptr;
				}

				llvm_type_t* arrayType = new llvm_type_array_t(this, elementT);
				if(!this->add_type(arrayType->name, arrayType))
				{
					delete arrayType;
					return nullptr;
				}
				
				return arrayType;
			}
			
			auto* type = this->scope->get_type(name, true);
			if(type == nullptr)
			{
				printf("The type '%s' is undefined.\n", name.cstr());
				return nullptr;
			}
			
			if(type->generics.size() != node->args.size())
			{
				printf("The generic type defination is not matched with the arguments. \n");
				return nullptr;
			}
			if(type->generics.size() > 0)
			{
				std::vector<llvm_type_t*> typeArgs = {};
				for(auto& arg : node->args)
				{
					auto* ty = this->encode_type(arg);
					if(ty == nullptr)
					{
						printf("Type Args is invalid.");
						return nullptr;
					}
					typeArgs.push_back(ty);
				}
				
				return type->resolve(typeArgs);
			}
			
			return type;
		}
		
		llvm::Value* encode_expr(ast_node_expr_t* node)
		{
			if(node == nullptr)
				return nullptr;
			
			switch (node->category)
			{
				case ast_category_t::EXPR_TRINARY:
					return this->encode_expr_trinary(dynamic_cast<ast_node_expr_trinary_t*>(node));
				case ast_category_t::EXPR_BINARY:
					return this->encode_expr_binary(dynamic_cast<ast_node_expr_binary_t*>(node));
				case ast_category_t::EXPR_UNARY:
					return this->encode_expr_unary(dynamic_cast<ast_node_expr_unary_t*>(node));
				case ast_category_t::LITERAL_INT:
					return this->encode_expr_int(dynamic_cast<ast_node_literal_int_t*>(node));
				case ast_category_t::LITERAL_FLOAT:
					return this->encode_expr_float(dynamic_cast<ast_node_literal_float_t*>(node));
				case ast_category_t::LITERAL_BOOL:
					return this->encode_expr_bool(dynamic_cast<ast_node_literal_bool_t*>(node));
				case ast_category_t::LITERAL_STRING:
					return this->encode_expr_string(dynamic_cast<ast_node_literal_string_t*>(node));
				case ast_category_t::SYMBOL_REF:
					return this->encode_expr_symbol_ref(dynamic_cast<ast_node_symbol_ref_t*>(node));
				case ast_category_t::FUNC_DEF:
					return this->encode_expr_func_def(dynamic_cast<ast_node_func_def_t*>(node));
				case ast_category_t::FUNC_REF:
					return this->encode_expr_func_ref(dynamic_cast<ast_node_func_ref_t*>(node));
				case ast_category_t::ARRAY_DEF:
					return this->encode_expr_array_def(dynamic_cast<ast_node_array_def_t*>(node));
				case ast_category_t::ARRAY_REF:
					return this->encode_expr_index_ref(dynamic_cast<ast_node_array_ref_t*>(node));
				case ast_category_t::OBJECT_DEF:
					return this->encode_expr_object_def(dynamic_cast<ast_node_object_def_t*>(node));
				case ast_category_t::OBJECT_REF:
					return this->encode_expr_object_ref(dynamic_cast<ast_node_object_ref_t*>(node));
				default:
					return nullptr;
			}
			return nullptr;
		}
		
		llvm::Value* encode_expr_trinary(struct ast_node_expr_trinary_t* node)
		{
			if(node == nullptr)
				return nullptr;
			
			llvm_function_t* func = this->func;
			llvm::IRBuilder<>& IR = this->func->IR;
			
			llvm::BasicBlock* trinary_begin = func->add_basic_block("trinary.begin");
			llvm::BasicBlock* trinary_true = func->add_basic_block("trinary.true");
			llvm::BasicBlock* trinary_false = func->add_basic_block("trinary.false");
			llvm::BasicBlock* trinary_end = func->add_basic_block("trinary.end");
			
			IR.CreateBr(trinary_begin);
			IR.SetInsertPoint(trinary_begin);
			llvm::Value* cond = this->encode_expr(node->cond);
			if(cond == nullptr)
				return nullptr;
			cond = func->get_value(cond);
			if(!cond->getType()->isIntegerTy(1))
			{
				printf("Condition must be a bool value.\n");
				return nullptr;
			}
			
			IR.CreateCondBr(cond, trinary_true, trinary_false);
			
			IR.SetInsertPoint(trinary_true);
			llvm::Value* trueV = this->encode_expr(node->branch_true);
			if(trueV == nullptr)
				return nullptr;
			IR.CreateBr(trinary_end);
			
			IR.SetInsertPoint(trinary_false);
			llvm::Value* falseV = this->encode_expr(node->branch_false);
			if(falseV == nullptr)
				return nullptr;
			IR.CreateBr(trinary_end);
			
			IR.SetInsertPoint(trinary_end);
			if(trueV->getType() != falseV->getType())
			{
				printf("Type of true-branch must be the same as false-branch.\n");
				return nullptr;
			}
			
			llvm::PHINode* phi = IR.CreatePHI(trueV->getType(), 2);
			phi->addIncoming(trueV, trinary_true);
			phi->addIncoming(falseV, trinary_false);
			
			return phi;
		}
		
		llvm::Value* encode_expr_binary(ast_node_expr_binary_t* node)
		{
			if(node == nullptr)
				return nullptr;
			
			auto left = this->encode_expr(node->left);
			auto right = this->encode_expr(node->right);
			if(left == nullptr || right == nullptr)
				return nullptr;
			
			auto lhs = this->func->get_value(left);
			auto rhs = this->func->get_value(right);
			
			switch (node->op)
			{
				case ast_binary_oper_t::OR:
					return this->encode_expr_binary_or(lhs, rhs);
				case ast_binary_oper_t::AND:
					return this->encode_expr_binary_and(lhs, rhs);
				case ast_binary_oper_t::EQ:
					return this->encode_expr_binary_eq(lhs, rhs);
				case ast_binary_oper_t::NE:
					return this->encode_expr_binary_ne(lhs, rhs);
				case ast_binary_oper_t::LE:
					return this->encode_expr_binary_le(lhs, rhs);
				case ast_binary_oper_t::GE:
					return this->encode_expr_binary_ge(lhs, rhs);
				case ast_binary_oper_t::LT:
					return this->encode_expr_binary_lt(lhs, rhs);
				case ast_binary_oper_t::GT:
					return this->encode_expr_binary_gt(lhs, rhs);
				case ast_binary_oper_t::ADD:
					return this->encode_expr_binary_add(lhs, rhs);
				case ast_binary_oper_t::SUB:
					return this->encode_expr_binary_sub(lhs, rhs);
				case ast_binary_oper_t::MUL:
					return this->encode_expr_binary_mul(lhs, rhs);
				case ast_binary_oper_t::DIV:
					return this->encode_expr_binary_div(lhs, rhs);
				case ast_binary_oper_t::MOD:
					return this->encode_expr_binary_mod(lhs, rhs);
				case ast_binary_oper_t::BIT_AND:
					return this->encode_expr_binary_bitand(lhs, rhs);
				case ast_binary_oper_t::BIT_OR:
					return this->encode_expr_binary_bitor(lhs, rhs);
				case ast_binary_oper_t::BIT_XOR:
					return this->encode_expr_binary_bitxor(lhs, rhs);
				case ast_binary_oper_t::SHIFT_L:
					return this->encode_expr_binary_bitshl(lhs, rhs);
				case ast_binary_oper_t::SHIFT_R:
					return this->encode_expr_binary_bitshr(lhs, rhs);
				default:
					return nullptr;
			}
		}
		
		llvm::Value* encode_expr_binary_or(llvm::Value* lhs, llvm::Value* rhs)
		{
			auto ltype = lhs->getType();
			auto rtype = rhs->getType();
			
			if((!ltype->isIntegerTy(1)) || (!rtype->isIntegerTy(1)))
			{
				printf("LHS or RHS is not bool value. \n");
				return nullptr;
			}
			
			return this->func->IR.CreateOr(lhs, rhs);
		}
		
		llvm::Value* encode_expr_binary_and(llvm::Value* lhs, llvm::Value* rhs)
		{
			auto ltype = lhs->getType();
			auto rtype = rhs->getType();
			
			if((!ltype->isIntegerTy(1)) || (!rtype->isIntegerTy(1)))
			{
				printf("LHS or RHS is not bool value. \n");
				return nullptr;
			}
			
			return this->func->IR.CreateAnd(lhs, rhs);
		}
		
		llvm::Value* encode_expr_binary_eq(llvm::Value* lhs, llvm::Value* rhs)
		{
			auto ltype = lhs->getType();
			auto rtype = rhs->getType();
			
			if(ltype->isIntegerTy() && rtype->isIntegerTy())
				return this->func->IR.CreateICmpEQ(lhs, rhs);
			
			if(ltype->isFloatingPointTy() && rtype->isFloatingPointTy())
				return this->func->IR.CreateFCmpOEQ(lhs, rhs);
			
			if(ltype->isPointerTy() && rtype->isPointerTy())
			{
				return this->func->IR.CreateICmpEQ(
					this->func->IR.CreatePtrToInt(lhs, llvm::Type::getInt64Ty(context)),
					this->func->IR.CreatePtrToInt(rhs, llvm::Type::getInt64Ty(context))
				);
			}
			
			if(ltype->isIntegerTy() && rtype->isFloatingPointTy())
			{
				return this->func->IR.CreateFCmpOEQ(
					this->func->IR.CreateSIToFP(lhs, llvm::Type::getDoubleTy(context)),
					rhs
				);
			}
			
			if(ltype->isFloatingPointTy() && rtype->isIntegerTy())
			{
				return this->func->IR.CreateFCmpOEQ(
					lhs,
					this->func->IR.CreateSIToFP(rhs, llvm::Type::getDoubleTy(context))
				);
			}
			
			printf("Type of LHS or RHS is invalid.\n");
			return nullptr;
		}
		
		llvm::Value* encode_expr_binary_ne(llvm::Value* lhs, llvm::Value* rhs)
		{
			auto ltype = lhs->getType();
			auto rtype = rhs->getType();
			
			if(ltype->isIntegerTy() && rtype->isIntegerTy())
				return this->func->IR.CreateICmpNE(lhs, rhs);
			
			if(ltype->isFloatingPointTy() && rtype->isFloatingPointTy())
				return this->func->IR.CreateFCmpONE(lhs, rhs);
			
			if(ltype->isPointerTy() && rtype->isPointerTy())
			{
				return this->func->IR.CreateICmpNE(
					this->func->IR.CreatePtrToInt(lhs, llvm::Type::getInt64Ty(context)),
					this->func->IR.CreatePtrToInt(rhs, llvm::Type::getInt64Ty(context))
				);
			}
			
			if(ltype->isIntegerTy() && rtype->isFloatingPointTy())
			{
				return this->func->IR.CreateFCmpONE(
					this->func->IR.CreateSIToFP(lhs, llvm::Type::getDoubleTy(context)),
					rhs
				);
			}
			
			if(ltype->isFloatingPointTy() && rtype->isIntegerTy())
			{
				return this->func->IR.CreateFCmpONE(
					lhs,
					this->func->IR.CreateSIToFP(rhs, llvm::Type::getDoubleTy(context))
				);
			}
			
			printf("Type of LHS or RHS is invalid.\n");
			return nullptr;
		}
		
		llvm::Value* encode_expr_binary_le(llvm::Value* lhs, llvm::Value* rhs)
		{
			auto ltype = lhs->getType();
			auto rtype = rhs->getType();
			
			if(ltype->isIntegerTy() && rtype->isIntegerTy())
				return this->func->IR.CreateICmpSLE(lhs, rhs);
			
			if(ltype->isFloatingPointTy() && rtype->isFloatingPointTy())
				return this->func->IR.CreateFCmpOLE(lhs, rhs);
			
			if(ltype->isPointerTy() && rtype->isPointerTy())
			{
				return this->func->IR.CreateICmpULE(
					this->func->IR.CreatePtrToInt(lhs, llvm::Type::getInt64Ty(context)),
					this->func->IR.CreatePtrToInt(rhs, llvm::Type::getInt64Ty(context))
				);
			}
			
			if(ltype->isIntegerTy() && rtype->isFloatingPointTy())
			{
				return this->func->IR.CreateFCmpOLE(
					this->func->IR.CreateSIToFP(lhs, llvm::Type::getDoubleTy(context)),
					rhs
				);
			}
			
			if(ltype->isFloatingPointTy() && rtype->isIntegerTy())
			{
				return this->func->IR.CreateFCmpOLE(
					lhs,
					this->func->IR.CreateSIToFP(rhs, llvm::Type::getDoubleTy(context))
				);
			}
			
			printf("Type of LHS or RHS is invalid.\n");
			return nullptr;
		}
		
		llvm::Value* encode_expr_binary_ge(llvm::Value* lhs, llvm::Value* rhs)
		{
			auto ltype = lhs->getType();
			auto rtype = rhs->getType();
			
			if(ltype->isIntegerTy() && rtype->isIntegerTy())
				return this->func->IR.CreateICmpSGE(lhs, rhs);
			
			if(ltype->isFloatingPointTy() && rtype->isFloatingPointTy())
				return this->func->IR.CreateFCmpOGE(lhs, rhs);
			
			if(ltype->isPointerTy() && rtype->isPointerTy())
			{
				return this->func->IR.CreateICmpUGE(
					this->func->IR.CreatePtrToInt(lhs, llvm::Type::getInt64Ty(context)),
					this->func->IR.CreatePtrToInt(rhs, llvm::Type::getInt64Ty(context))
				);
			}
			
			if(ltype->isIntegerTy() && rtype->isFloatingPointTy())
			{
				return this->func->IR.CreateFCmpOGE(this->func->IR.CreateSIToFP(lhs, llvm::Type::getDoubleTy(context)), rhs);
			}
			
			if(ltype->isFloatingPointTy() && rtype->isIntegerTy())
			{
				return this->func->IR.CreateFCmpOGE(lhs, this->func->IR.CreateSIToFP(rhs, llvm::Type::getDoubleTy(context)));
			}
			
			printf("Type of LHS or RHS is invalid.\n");
			return nullptr;
		}
		
		llvm::Value* encode_expr_binary_lt(llvm::Value* lhs, llvm::Value* rhs)
		{
			auto ltype = lhs->getType();
			auto rtype = rhs->getType();
			
			if(ltype->isIntegerTy() && rtype->isIntegerTy())
				return this->func->IR.CreateICmpSLT(lhs, rhs);
			
			if(ltype->isFloatingPointTy() && rtype->isFloatingPointTy())
				return this->func->IR.CreateFCmpOLT(lhs, rhs);
			
			if(ltype->isPointerTy() && rtype->isPointerTy())
			{
				return this->func->IR.CreateICmpULT(this->func->IR.CreatePtrToInt(lhs, llvm::Type::getInt64Ty(context)), this->func->IR.CreatePtrToInt(rhs, llvm::Type::getInt64Ty(context)));
			}
			
			if(ltype->isIntegerTy() && rtype->isFloatingPointTy())
			{
				return this->func->IR.CreateFCmpOLT(this->func->IR.CreateSIToFP(lhs, llvm::Type::getDoubleTy(context)), rhs);
			}
			
			if(ltype->isFloatingPointTy() && rtype->isIntegerTy())
			{
				return this->func->IR.CreateFCmpOLT(lhs, this->func->IR.CreateSIToFP(rhs, llvm::Type::getDoubleTy(context)));
			}
			
			printf("Type of LHS or RHS is invalid.\n");
			return nullptr;
		}
		
		llvm::Value* encode_expr_binary_gt(llvm::Value* lhs, llvm::Value* rhs)
		{
			auto ltype = lhs->getType();
			auto rtype = rhs->getType();
			
			if(ltype->isIntegerTy() && rtype->isIntegerTy())
				return this->func->IR.CreateICmpSGT(lhs, rhs);
			
			if(ltype->isFloatingPointTy() && rtype->isFloatingPointTy())
				return this->func->IR.CreateFCmpOGT(lhs, rhs);
			
			if(ltype->isPointerTy() && rtype->isPointerTy())
			{
				return this->func->IR.CreateICmpUGT(this->func->IR.CreatePtrToInt(lhs, llvm::Type::getInt64Ty(context)), this->func->IR.CreatePtrToInt(rhs, llvm::Type::getInt64Ty(context)));
			}
			
			if(ltype->isIntegerTy() && rtype->isFloatingPointTy())
			{
				return this->func->IR.CreateFCmpOGT(this->func->IR.CreateSIToFP(lhs, llvm::Type::getDoubleTy(context)), rhs);
			}
			
			if(ltype->isFloatingPointTy() && rtype->isIntegerTy())
			{
				return this->func->IR.CreateFCmpOGT(lhs, this->func->IR.CreateSIToFP(rhs, llvm::Type::getDoubleTy(context)));
			}
			
			printf("Type of LHS or RHS is invalid.\n");
			return nullptr;
		}
		
		llvm::Value* encode_expr_binary_add(llvm::Value* lhs, llvm::Value* rhs)
		{
			auto ltype = lhs->getType();
			auto rtype = rhs->getType();
			
			if(ltype->isIntegerTy() && rtype->isIntegerTy())
				return this->func->IR.CreateAdd(lhs, rhs);
			
			if(ltype->isFloatingPointTy() && rtype->isFloatingPointTy())
				return this->func->IR.CreateFAdd(lhs, rhs);
			
			if(ltype->isIntegerTy() && rtype->isFloatingPointTy())
			{
				return this->func->IR.CreateFAdd(this->func->IR.CreateSIToFP(lhs, llvm::Type::getDoubleTy(context)), rhs);
			}
			
			if(ltype->isFloatingPointTy() && rtype->isIntegerTy())
			{
				return this->func->IR.CreateFAdd(lhs, this->func->IR.CreateSIToFP(rhs, llvm::Type::getDoubleTy(context)));
			}
			
			printf("Type of LHS or RHS is invalid.\n");
			return nullptr;
		}
		
		llvm::Value* encode_expr_binary_sub(llvm::Value* lhs, llvm::Value* rhs)
		{
			auto ltype = lhs->getType();
			auto rtype = rhs->getType();
			
			if(ltype->isIntegerTy() && rtype->isIntegerTy())
				return this->func->IR.CreateSub(lhs, rhs);
			
			if(ltype->isFloatingPointTy() && rtype->isFloatingPointTy())
				return this->func->IR.CreateFSub(lhs, rhs);
			
			if(ltype->isIntegerTy() && rtype->isFloatingPointTy())
			{
				return this->func->IR.CreateFSub(this->func->IR.CreateSIToFP(lhs, llvm::Type::getDoubleTy(context)), rhs);
			}
			
			if(ltype->isFloatingPointTy() && rtype->isIntegerTy())
			{
				return this->func->IR.CreateFSub(lhs, this->func->IR.CreateSIToFP(rhs, llvm::Type::getDoubleTy(context)));
			}
			
			printf("Type of LHS or RHS is invalid.\n");
			return nullptr;
		}
		
		llvm::Value* encode_expr_binary_mul(llvm::Value* lhs, llvm::Value* rhs)
		{
			auto ltype = lhs->getType();
			auto rtype = rhs->getType();
			
			if(ltype->isIntegerTy() && rtype->isIntegerTy())
				return this->func->IR.CreateMul(lhs, rhs);
			
			if(ltype->isFloatingPointTy() && rtype->isFloatingPointTy())
				return this->func->IR.CreateFMul(lhs, rhs);
			
			if(ltype->isIntegerTy() && rtype->isFloatingPointTy())
			{
				return this->func->IR.CreateFMul(this->func->IR.CreateSIToFP(lhs, llvm::Type::getDoubleTy(context)), rhs);
			}
			
			if(ltype->isFloatingPointTy() && rtype->isIntegerTy())
			{
				return this->func->IR.CreateFMul(lhs, this->func->IR.CreateSIToFP(rhs, llvm::Type::getDoubleTy(context)));
			}
			
			printf("Type of LHS or RHS is invalid.\n");
			return nullptr;
		}
		
		llvm::Value* encode_expr_binary_div(llvm::Value* lhs, llvm::Value* rhs)
		{
			auto ltype = lhs->getType();
			auto rtype = rhs->getType();
			
			if(ltype->isIntegerTy() && rtype->isIntegerTy())
				return this->func->IR.CreateSDiv(lhs, rhs);
			
			if(ltype->isFloatingPointTy() && rtype->isFloatingPointTy())
				return this->func->IR.CreateFMul(lhs, rhs);
			
			if(ltype->isIntegerTy() && rtype->isFloatingPointTy())
			{
				return this->func->IR.CreateFDiv(this->func->IR.CreateSIToFP(lhs, llvm::Type::getDoubleTy(context)), rhs);
			}
			
			if(ltype->isFloatingPointTy() && rtype->isIntegerTy())
			{
				return this->func->IR.CreateFDiv(lhs, this->func->IR.CreateSIToFP(rhs, llvm::Type::getDoubleTy(context)));
			}
			
			printf("Type of LHS or RHS is invalid.\n");
			return nullptr;
		}
		
		llvm::Value* encode_expr_binary_mod(llvm::Value* lhs, llvm::Value* rhs)
		{
			auto ltype = lhs->getType();
			auto rtype = rhs->getType();
			
			if(ltype->isIntegerTy() && rtype->isIntegerTy())
				return this->func->IR.CreateSRem(lhs, rhs);
			
			if(ltype->isFloatingPointTy() && rtype->isFloatingPointTy())
				return this->func->IR.CreateFRem(lhs, rhs);
			
			if(ltype->isIntegerTy() && rtype->isFloatingPointTy())
			{
				return this->func->IR.CreateFRem(this->func->IR.CreateSIToFP(lhs, llvm::Type::getDoubleTy(context)), rhs);
			}
			
			if(ltype->isFloatingPointTy() && rtype->isIntegerTy())
			{
				return this->func->IR.CreateFRem(lhs, this->func->IR.CreateSIToFP(rhs, llvm::Type::getDoubleTy(context)));
			}
			
			printf("Type of LHS or RHS is invalid.\n");
			return nullptr;
		}
		
		llvm::Value* encode_expr_binary_bitand(llvm::Value* lhs, llvm::Value* rhs)
		{
			auto ltype = lhs->getType();
			auto rtype = rhs->getType();
			
			if(ltype->isIntegerTy() && rtype->isIntegerTy())
				return this->func->IR.CreateAnd(lhs, rhs);
			
			printf("Type of LHS or RHS is invalid.\n");
			return nullptr;
		}
		
		llvm::Value* encode_expr_binary_bitor(llvm::Value* lhs, llvm::Value* rhs)
		{
			auto ltype = lhs->getType();
			auto rtype = rhs->getType();
			
			if(ltype->isIntegerTy() && rtype->isIntegerTy())
				return this->func->IR.CreateOr(lhs, rhs);
			
			printf("Type of LHS or RHS is invalid.\n");
			return nullptr;
		}
		
		llvm::Value* encode_expr_binary_bitxor(llvm::Value* lhs, llvm::Value* rhs)
		{
			auto ltype = lhs->getType();
			auto rtype = rhs->getType();
			
			if(ltype->isIntegerTy() && rtype->isIntegerTy())
				return this->func->IR.CreateXor(lhs, rhs);
			
			printf("Type of LHS or RHS is invalid.\n");
			return nullptr;
		}
		
		llvm::Value* encode_expr_binary_bitshl(llvm::Value* lhs, llvm::Value* rhs)
		{
			auto ltype = lhs->getType();
			auto rtype = rhs->getType();
			
			if(ltype->isIntegerTy() && rtype->isIntegerTy())
				return this->func->IR.CreateShl(lhs, rhs);
			
			printf("Type of LHS or RHS is invalid.\n");
			return nullptr;
		}
		
		llvm::Value* encode_expr_binary_bitshr(llvm::Value* lhs, llvm::Value* rhs)
		{
			auto ltype = lhs->getType();
			auto rtype = rhs->getType();
			
			if(ltype->isIntegerTy() && rtype->isIntegerTy())
			{
				// 逻辑右移：在左边补 0
				// 算术右移：在左边补 符号位
				// 我们采用逻辑右移
				return this->func->IR.CreateLShr(lhs, rhs);
			}
			
			printf("Type of LHS or RHS is invalid.\n");
			return nullptr;
		}
		
		llvm::Value* encode_expr_unary(ast_node_expr_unary_t* node)
		{
			if(node == nullptr)
				return nullptr;
			
			auto right = this->encode_expr(node->right);
			if(right == nullptr)
				return nullptr;
			
			auto rhs = this->func->get_value( right);
			
			switch (node->op)
			{
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
		
		llvm::Value* encode_expr_unary_neg(llvm::Value* rhs)
		{
			auto rtype = rhs->getType();
			
			if(rtype->isIntegerTy())
				return this->func->IR.CreateNeg(rhs);
			
			if(rtype->isFloatingPointTy())
				return this->func->IR.CreateFNeg(rhs);
			
			printf("Type of RHS is invalid.\n");
			return nullptr;
		}
		
		llvm::Value* encode_expr_unary_not(llvm::Value* rhs)
		{
			auto rtype = rhs->getType();
			
			if(rtype->isIntegerTy() && rtype->getIntegerBitWidth() == 1)
				return this->func->IR.CreateNot(rhs);
			
			printf("Type of RHS is invalid.\n");
			return nullptr;
		}
		
		llvm::Value* encode_expr_unary_flip(llvm::Value* rhs)
		{
			auto rtype = rhs->getType();
			
			if(rtype->isIntegerTy())
			{
				return this->func->IR.CreateXor(rhs, llvm::ConstantInt::get(rtype, llvm::APInt(rtype->getIntegerBitWidth(), 0xFFFFFFFF)));
			}
			printf("Type of RHS is invalid.\n");
			return nullptr;
		}
		
		llvm::Value* encode_expr_int(ast_node_literal_int_t* node)
		{
			if(node == nullptr)
				return nullptr;
			
			u64_t vals = *((u64_t*) &(node->value));
			u32_t bits = vals>0xFFFFFFFF ? 64 : 32;
			
			return llvm::ConstantInt::get(context, llvm::APInt(bits, node->value));
		}
		
		llvm::Value* encode_expr_float(ast_node_literal_float_t* node)
		{
			if(node == nullptr)
				return nullptr;
			
			return llvm::ConstantFP::get(context, llvm::APFloat(node->value));
		}
		
		llvm::Value* encode_expr_bool(ast_node_literal_bool_t* node)
		{
			if(node == nullptr)
				return nullptr;
			return llvm::ConstantInt::getBool(context, node->value);
		}
		
		llvm::Value* encode_expr_string(ast_node_literal_string_t* node)
		{
			if(node == nullptr)
				return nullptr;
			
			auto str = func->string_make(node->value.cstr());
			return str;
		}
		
		llvm::Value* encode_expr_symbol_ref(ast_node_symbol_ref_t* node)
		{
			if(node == nullptr)
				return nullptr;
			
			auto* symbol = this->scope->getSymbol(node->name, true);
			if(symbol == nullptr)
			{
				printf("Symbol '%s' is undefined.\n", node->name.cstr());
				return nullptr;
			}
			
			// local-value-ref
			if(symbol->scope->func == this->func)
			{
				return symbol->value;
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
			
			return symbol->value;
		}
		
		llvm::Value* encode_expr_func_def(ast_node_func_def_t* node)
		{
			if(node == nullptr)
				return nullptr;
			
			auto* retType = this->encode_type(node->rtype);
			if(retType == nullptr)
				return nullptr;
			
			std::vector<llvm::Type*> argTypes;
			for (auto& arg: node->args)
			{
				auto* argType = this->encode_type(arg.type);
				if(argType == nullptr)
					return nullptr;
				if(argType->is_reference_type())
					argTypes.push_back(argType->handle->getPointerTo());
				else
					argTypes.push_back(argType->handle);
			}
			
			auto newFunc = this->module->add_func("", retType->handle, argTypes, false);
			
			auto oldFunc = this->func;
			auto oldIB = this->func->IR.GetInsertBlock();
			
			this->func = newFunc;
			
			this->pushScope(newFunc);
			{
				llvm::BasicBlock* entry = newFunc->add_basic_block("entry");
				this->func->IR.SetInsertPoint(entry);
				
				// self
				auto self = newFunc->handle;
				this->scope->addSymbol("self", self);
				
				// args
				for (size_t index = 0; index<node->args.size(); index++)
				{
					const char* name = node->args.at(index).name.cstr();
					auto arg = newFunc->handle->getArg(index + 1);
					arg->setName(name);
					
					if(!this->scope->addSymbol(name, arg))
					{
						printf("The symbol name '%s' is already existed.\n", name);
						return nullptr;
					}
				}
				
				// body
				for (auto& stmt: node->body)
				{
					if(!this->encode_stmt(stmt))
						return nullptr;
				}
				auto& lastOp = this->func->IR.GetInsertBlock()->back();
				if(!lastOp.isTerminator())
				{
					if(newFunc->handle->getReturnType()->isVoidTy())
						this->func->IR.CreateRetVoid();
					else
						this->func->IR.CreateRet(module->get_default_value(retType));
				}
			}
			this->popScope();
			
			this->func = oldFunc;
			this->func->IR.SetInsertPoint(oldIB);
			
			return newFunc->handle;
		}
		
		llvm::Value* encode_expr_func_ref(ast_node_func_ref_t* node)
		{
			if(node == nullptr)
				return nullptr;
			
			llvm::Value* funcExpr = this->encode_expr(node->func);
			if(funcExpr == nullptr)
			{
				printf("The function is undefined.\n");
				return nullptr;
			}
			
			auto funcPtr = this->func->get_value( funcExpr);
			llvm::FunctionType* funcType = nullptr;
			if(funcPtr->getType()->isFunctionTy())
			{
				funcType = llvm::cast<llvm::FunctionType>(funcPtr->getType());
			}
			else if(funcPtr->getType()->isPointerTy() && funcPtr->getType()->getPointerElementType()->isFunctionTy())
			{
				funcType = llvm::cast<llvm::FunctionType>(funcPtr->getType()->getPointerElementType());
			}
			else
			{
				printf("Invalid function type.\n");
				return nullptr;
			}
			
			std::vector<llvm::Value*> args;
			for (auto i = 0; i<node->args.size(); i++)
			{
				auto* paramT = funcType->getParamType(i);
				auto* argV = this->encode_expr(node->args.at(i));
				if(argV == nullptr)
					return nullptr;
				
				argV = this->func->get_value( argV);
				
				if(paramT != argV->getType())
				{
					if(paramT == module->type_cstr)
					{
						argV = func->cstr_from_value(argV);
					}
					if(paramT == module->type_string_ptr)
					{
						argV = func->string_from_value(argV);
					}
					else if(!argV->getType()->canLosslesslyBitCastTo(paramT))
					{
						printf("The type of param[%d] can't cast to the param type of function.\n", i);
						return nullptr;
					}
				}
				
				args.push_back(argV);
			}
			
			llvm::Value* retval = this->func->IR.CreateCall(funcType, funcPtr, args);
			return retval;
		}
		
		llvm::Value* encode_expr_array_def(ast_node_array_def_t* node)
		{
			if(node == nullptr)
				return nullptr;
			
			std::vector<llvm::Value*> arrayElements;
			llvm::Type* arrayElementType = nullptr;
			for (auto element: node->elements)
			{
				auto elementV = this->encode_expr(element);
				if(elementV == nullptr)
					return nullptr;
				elementV = this->func->get_value( elementV);
				
				auto elementT = elementV->getType();
				if(arrayElementType == nullptr)
				{
					arrayElementType = elementT;
				}
				
				else if(arrayElementType != elementT)
				{
					printf("The type of some elements is not same as others.\n");
					return nullptr;
				}
				
				arrayElements.push_back(elementV);
			}
			
			if(arrayElements.empty() || arrayElementType == nullptr)
				arrayElementType = llvm::Type::getInt32Ty(context);
			
			auto arrayT = new llvm_type_array_t(*module, arrayElementType);
			auto arrayP = module->make(func, this->func->IR, arrayT);
			module->array_set(scope->func, this->func->IR, arrayP, arrayElements);
			
			return arrayP;
		}
		
		llvm::Value* encode_expr_index_ref(ast_node_array_ref_t* node)
		{
			if(node == nullptr)
				return nullptr;
			
			auto objV = this->encode_expr(node->obj);
			auto keyV = this->encode_expr(node->key);
			if(objV == nullptr || keyV == nullptr)
				return nullptr;
			
			objV = this->func->get_value( objV);
			keyV = this->func->get_value( keyV);
			auto objT = objV->getType();
			auto keyT = keyV->getType();
			
			if(module->is_schema_array(objT))
			{
				if(keyT->isIntegerTy())
				{
					return module->array_get(scope->func, this->func->IR, objV, keyV);
				}
				else
				{
					printf("The type of index is invalid.\n");
					return nullptr;
				}
			}
			else if(objT == module->type_string_ptr)
			{
				if(keyT->isIntegerTy())
				{
					return module->string_get_char(scope->func, this->func->IR, objV, keyV);
				}
				else
				{
					printf("The type of index is invalid.\n");
					return nullptr;
				}
			}
			else
			{
				printf("Index-Access is not defined on the object.\n");
				return nullptr;
			}
		}
		
		llvm::Value* encode_expr_object_def(ast_node_object_def_t* node)
		{
			if(node == nullptr)
				return nullptr;
			
			auto* structType = this->encode_type(node->type);
			if(structType == nullptr)
				return nullptr;
			
			auto* structInfo = module->get_struct(structType);
			if(structInfo == nullptr)
				return nullptr;
			
			for (auto& objectMember: node->members)
			{
				auto structMember = structInfo->get_member(objectMember.first);
				if(structMember == nullptr)
				{
					printf("Object member '%s' is not defined in struct.\n", objectMember.first.cstr());
					return nullptr;
				}
				// todo: check object-member-type equals to struct-member-type.
			}
			
			// make object instance
			llvm::Value* instance = module->make(this->scope->func, this->func->IR, structType);
			
			for (u32_t index = 0; index<structInfo->members.size(); index++)
			{
				auto& structMember = structInfo->members.at(index);
				auto objectMember = node->members.find(structMember->name);
				llvm::Value* memV = nullptr;
				if(objectMember != node->members.end())
				{
					memV = this->encode_expr(objectMember->second);
					if(memV == nullptr)
						return nullptr;
					memV = this->func->get_value( memV);
				}
				else
				{
					memV = structMember->value != nullptr ? structMember->value : module->get_default_value(structMember->type);
				}
				
				if(memV)
				{
					String memN = String::format("this.%s", structMember->name.cstr());
					llvm::Value* memP = this->func->IR.CreateStructGEP(structType, instance, index, memN.cstr());
					this->func->IR.CreateStore(memV, memP);
				}
			}
			
			return instance;
		}
		
		llvm::Value* encode_expr_object_ref(ast_node_object_ref_t* node)
		{
			if(node == nullptr)
				return nullptr;
			
			auto objV = this->encode_expr(node->obj);
			auto keyV = this->func->IR.CreateGlobalString(node->key.cstr());
			if(objV == nullptr || keyV == nullptr)
				return nullptr;
			
			objV = this->func->get_value( objV);
			auto objT = objV->getType();
			if(!(objT->isPointerTy() && objT->getPointerElementType()->isStructTy()))
			{
				printf("The value is not a object reference.\n");
				return nullptr;
			}
			
			auto structInfo = module->get_struct(objT->getPointerElementType());
			if(structInfo == nullptr)
			{
				printf("Can't find the type of this value.\n");
				return nullptr;
			}
			
			
			auto index = structInfo->get_member_index(node->key);
			if(index == -1)
			{
				printf("The object doesn't have a member named '%s'. \n", node->key.cstr());
				return nullptr;
			}
			
			auto structType = structInfo->type;
			llvm::Value* value = this->func->IR.CreateStructGEP(structType, objV, index);
			return value;
		}
		
		bool encode_stmt(ast_node_stmt_t* node)
		{
			if(node == nullptr)
				return false;
			
			switch (node->category)
			{
				case ast_category_t::STRUCT_DEF:
					return this->encode_stmt_struct_def(dynamic_cast<ast_node_struct_def_t*>(node));
				case ast_category_t::ENUM_DEF:
					return this->encode_stmt_enum_def(dynamic_cast<ast_node_enum_def_t*>(node));
				case ast_category_t::PROC_DEF:
					return this->encode_stmt_proc_def(dynamic_cast<ast_node_proc_def_t*>(node));
				case ast_category_t::SYMBOL_DEF:
					return this->encode_stmt_symbol_def(dynamic_cast<ast_node_symbol_def_t*>(node));
				case ast_category_t::BREAK:
					return this->encode_stmt_break(dynamic_cast<ast_node_break_t*>(node));
				case ast_category_t::CONTINUE:
					return this->encode_stmt_continue(dynamic_cast<ast_node_continue_t*>(node));
				case ast_category_t::RETURN:
					return this->encode_stmt_return(dynamic_cast<ast_node_return_t*>(node));
				case ast_category_t::IF:
					return this->encode_stmt_if(dynamic_cast<ast_node_if_t*>(node));
				case ast_category_t::LOOP:
					return this->encode_stmt_loop(dynamic_cast<ast_node_loop_t*>(node));
				case ast_category_t::BLOCK:
					return this->encode_stmt_block(dynamic_cast<ast_node_block_t*>(node));
				case ast_category_t::ASSIGN:
					return this->encode_stmt_assign(dynamic_cast<ast_node_assign_t*>(node));
				case ast_category_t::INVOKE:
					return this->encode_stmt_invoke(dynamic_cast<ast_node_invoke_t*>(node));
				default:
					return false;
			}
			
			return false;
		}
		
		bool encode_stmt_struct_def(ast_node_struct_def_t* node)
		{
			if(node == nullptr)
				return false;
			
			auto* thisInstanceInfo = module->new_struct(node->name);
			
			for (const auto& thisMember: node->members)
			{
				const String& memName = thisMember.name;
				if(thisInstanceInfo->get_member(memName) != nullptr)
				{
					printf("The member named '%s' is already exists.\n", memName.cstr());
					return false;
				}
				
				auto memType = this->encode_type(thisMember.type);
				if(memType == nullptr)
					return false;
				
				auto memValue = this->encode_expr(thisMember.value);
				if(memValue == nullptr)
				{
					memValue = module->get_default_value(memType);
				}
				
				thisInstanceInfo->add_member(memName, memType, memValue);
			}
			
			thisInstanceInfo->resolve();
			if(!this->scope->addSchema(thisInstanceInfo->name, thisInstanceInfo->type))
			{
				printf("There is a same schema named %s in this scope.\n", thisInstanceInfo->name.cstr());
				return false;
			}
			
			return true;
		}
		
		bool encode_stmt_enum_def(struct ast_node_enum_def_t* node)
		{
			if(node == nullptr)
				return false;
			
			const String name = node->name;
			
			auto* thisInstanceInfo = module->new_struct(node->name);
			thisInstanceInfo->add_member("value", module->type_i32);
			thisInstanceInfo->resolve();
			
			const String staticTypePrefix = "$_Static";
			auto* thisStaticInfo = module->new_struct(staticTypePrefix + node->name);
			for (const auto& thisMember: node->members)
			{
				auto memName = thisMember.first;
				if(thisStaticInfo->get_member(memName) != nullptr)
				{
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
			
			if(!this->scope->addSchema(thisStaticInfo->name, thisStaticInfo->type) || !this->scope->addSchema(thisInstanceInfo->name, thisInstanceInfo->type))
			{
				printf("There is a same schema named %s in this scope.\n", thisInstanceInfo->name.cstr());
				return false;
			}
			
			// make static object
			llvm::Value* staticV = module->make(this->scope->func, this->func->IR, thisStaticInfo->type);
			for (u32_t index = 0; index<thisStaticInfo->members.size(); index++)
			{
				auto& mem = thisStaticInfo->members.at(index);
				auto memV = this->func->IR.CreateLoad(mem->value);
				
				String memN = String::format("%s.%s", name.cstr(), mem->name.cstr());
				llvm::Value* memP = this->func->IR.CreateStructGEP(thisStaticInfo->type, staticV, index, memN.cstr());
				this->func->IR.CreateStore(memV, memP);
			}
			if(!this->scope->addSymbol(name, staticV))
			{
				printf("There is a same symbol named %s in this scope.\n", name.cstr());
				return false;
			}
			
			return true;
		}
		
		bool encode_stmt_proc_def(ast_node_proc_def_t* node)
		{
			if(node == nullptr)
				return false;
			
			if(this->scope->getSchema(node->name, false) != nullptr)
				return false;
			
			auto* retType = this->encode_type(node->type);
			
			std::vector<llvm::Type*> argTypes;
			for (auto arg: node->args)
			{
				auto* argType = this->encode_type(arg.second);
				if(argType == nullptr)
					return false;
				argTypes.push_back(argType);
			}
			
			llvm::FunctionType* procType = llvm::FunctionType::get(retType, argTypes, false);
			this->scope->addSchema(node->name, procType->getPointerTo());
			
			return true;
		}
		
		bool encode_stmt_symbol_def(struct ast_node_symbol_def_t* node)
		{
			if(node == nullptr)
				return false;
			
			if(this->scope->getSymbol(node->name, false) != nullptr)
			{
				printf("The symbol '%s' is undefined.", node->name.cstr());
				return false;
			}
			
			auto type = this->encode_type(node->type);
			auto expr = this->encode_expr(node->value);
			if(expr == nullptr)
				return false;
			
			auto value = this->func->get_value( expr);
			
			llvm::Type* stype = nullptr;
			llvm::Type* vtype = value->getType();
			if(type != nullptr)
			{
				stype = type;
				do
				{
					if(stype == vtype)
						break;
					if(vtype->canLosslesslyBitCastTo(stype))
						break;
					if(vtype->isPointerTy() && vtype->getPointerElementType() == stype)
					{
						stype = type = vtype;
						break;
					}
					
					// TODO: 校验类型合法性, 值类型是否遵循标记类型
					
					printf("Type of value can not cast to the type of symbol.\n");
					return false;
				} while (false);
			}
			else
			{
				stype = vtype;
			}
			
			if(stype->isVoidTy())
			{
				printf("Void-Type can't assign to a symbol.\n");
				return false;
			}
			
			llvm::Value* symbol = this->func->IR.CreateAlloca(stype);
			this->func->IR.CreateStore(value, symbol);
			symbol = this->func->ref_value(symbol);
			symbol->setName(node->name.cstr());
			
			if(!scope->addSymbol(node->name, symbol))
			{
				printf("There is a symbol named %s in this scope.\n", node->name.cstr());
				return false;
			}
			
			return true;
		}
		
		bool encode_stmt_break(struct ast_node_break_t* node)
		{
			if(node == nullptr)
				return false;
			
			if(this->breakPoint == nullptr)
				return false;
			
			this->func->IR.CreateBr(this->breakPoint);
			
			return true;
		}
		
		bool encode_stmt_continue(struct ast_node_continue_t* node)
		{
			if(node == nullptr)
				return false;
			
			if(this->continuePoint == nullptr)
				return false;
			
			this->func->IR.CreateBr(this->continuePoint);
			
			return true;
		}
		
		bool encode_stmt_return(struct ast_node_return_t* node)
		{
			if(node == nullptr)
				return false;
			
			auto expectedRetType = this->func->type->getReturnType();
			
			if(node->value == nullptr)
			{
				if(!expectedRetType->isVoidTy())
				{
					printf("The function must return a value.\n");
					return false;
				}
				
				this->func->IR.CreateRetVoid();
				return true;
			}
			
			auto expr = this->encode_expr(node->value);
			if(expr == nullptr)
			{
				printf("Invalid ret value.\n");
				return false;
			}
			auto value = this->func->get_value( expr);
			auto actureRetType = value->getType();
			if(actureRetType != expectedRetType && !actureRetType->canLosslesslyBitCastTo(expectedRetType))
			{
				printf("The type of return value can't cast to return type of function.\n");
				return false;
			}
			
			this->func->IR.CreateRet(value);
			
			return true;
		}
		
		bool encode_stmt_if(struct ast_node_if_t* node)
		{
			if(node == nullptr)
				return false;
			
			llvm::BasicBlock* if_true = this->func->add_basic_block("if.true");
			llvm::BasicBlock* if_false = this->func->add_basic_block("if.false");
			llvm::BasicBlock* if_end = this->func->add_basic_block("if.end");
			
			auto condV = this->encode_expr(node->cond);
			if(condV == nullptr)
				return false;
			condV = this->func->get_value( condV);
			if(!condV->getType()->isIntegerTy(1))
			{
				printf("The label 'if.cond' need a bool value.\n");
				return false;
			}
			this->func->IR.CreateCondBr(condV, if_true, if_false);
			
			// if-true
			this->func->IR.SetInsertPoint(if_true);
			if(node->branch_true != nullptr)
			{
				if(!this->encode_stmt(node->branch_true))
					return false;
				auto lastBlock = this->func->IR.GetInsertBlock();
				if(lastBlock != if_true && !lastBlock->back().isTerminator())
				{
					this->func->IR.CreateBr(if_end);
				}
			}
			if(!if_true->back().isTerminator())
			{
				this->func->IR.CreateBr(if_end);
			}
			
			// if-false
			this->func->IR.SetInsertPoint(if_false);
			if(node->branch_false != nullptr)
			{
				if(!this->encode_stmt(node->branch_false))
					return false;
				auto lastBlock = this->func->IR.GetInsertBlock();
				if(lastBlock != if_false && !lastBlock->back().isTerminator())
				{
					this->func->IR.CreateBr(if_end);
				}
			}
			if(!if_false->back().isTerminator())
			{
				this->func->IR.CreateBr(if_end);
			}
			
			this->func->IR.SetInsertPoint(if_end);
			
			return true;
		}
		
		bool encode_stmt_loop(struct ast_node_loop_t* node)
		{
			if(node == nullptr)
				return false;
			
			this->pushScope();
			
			llvm::BasicBlock* loop_cond = this->func->add_basic_block("loop.cond");
			llvm::BasicBlock* loop_step = this->func->add_basic_block("loop.step");
			llvm::BasicBlock* loop_body = this->func->add_basic_block("loop.body");
			llvm::BasicBlock* loop_end = this->func->add_basic_block("loop.end");
			
			auto oldContinuePoint = this->continuePoint;
			auto oldBreakPoint = this->breakPoint;
			this->continuePoint = loop_step;
			this->breakPoint = loop_end;
			
			if(!this->encode_stmt(node->init))
				return false;
			this->func->IR.CreateBr(loop_cond);
			
			this->func->IR.SetInsertPoint(loop_cond);
			auto condV = this->encode_expr(node->cond);
			if(condV == nullptr)
				return false;
			condV = this->func->get_value( condV);
			if(!condV->getType()->isIntegerTy(1))
			{
				printf("The label 'for.cond' need a bool value.\n");
				return false;
			}
			this->func->IR.CreateCondBr(condV, loop_body, loop_end);
			
			this->func->IR.SetInsertPoint(loop_body);
			if(node->body != nullptr)
			{
				if(!this->encode_stmt(node->body))
					return false;
				auto& lastOp = loop_body->back();
				if(!lastOp.isTerminator())
				{
					this->func->IR.CreateBr(loop_step);
				}
			}
			if(!loop_body->back().isTerminator())
			{
				this->func->IR.CreateBr(loop_step);
			}
			
			this->func->IR.SetInsertPoint(loop_step);
			if(!this->encode_stmt(node->step))
				return false;
			this->func->IR.CreateBr(loop_cond);
			
			this->func->IR.SetInsertPoint(loop_end);
			
			this->continuePoint = oldContinuePoint;
			this->breakPoint = oldBreakPoint;
			
			this->popScope();
			
			return true;
		}
		
		bool encode_stmt_block(struct ast_node_block_t* node)
		{
			if(node == nullptr)
				return false;
			
			this->pushScope();
			
			for (auto& stmt: node->stmts)
			{
				if(!this->encode_stmt(stmt))
					return false;
			}
			
			this->popScope();
			
			return true;
		}
		
		bool encode_stmt_assign(struct ast_node_assign_t* node)
		{
			if(node == nullptr)
				return false;
			
			auto left = this->encode_expr(node->left);
			auto right = this->encode_expr(node->right);
			if(left == nullptr || right == nullptr)
				return false;
			
			auto ptr = llvm_model_t::ref_value(this->func->IR, left);
			auto val = this->func->get_value( right);
			this->func->IR.CreateStore(val, ptr);
			
			return true;
		}
		
		bool encode_stmt_invoke(struct ast_node_invoke_t* node)
		{
			if(node == nullptr)
				return false;
			
			if(!this->encode_expr_func_ref(node->expr))
				return false;
			
			return true;
		}
	};
	
	llvm_module_t* llvm_encode(llvm::LLVMContext& context, ast_node_module_t* module)
	{
		auto* coder = new llvm_coder_t(context, module->name);
		if(!coder->encode_module(module))
		{
			return nullptr;
		}
		return coder;
	}
}
