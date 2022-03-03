#include "coder.h"
#include "models.h"

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
	
	struct llvm_coder_t
	{
		llvm::LLVMContext& context;
		llvm::IRBuilder<> builder;
		
		llvm_module_t* module;
		
		llvm_scope_t* scope;
		llvm_func_t* func;
		
		llvm::BasicBlock* continuePoint;
		llvm::BasicBlock* breakPoint;
		
		explicit llvm_coder_t(llvm::LLVMContext& context)
			: context(context), builder(context), module(nullptr)
		{
			this->scope = nullptr;
			this->func = nullptr;
			this->continuePoint = nullptr;
			this->breakPoint = nullptr;
		}
		
		virtual ~llvm_coder_t()
		{
		}
		
		void pushScope(llvm::Function* f = nullptr)
		{
			this->scope = this->scope->addChild(f);
		}
		
		void popScope()
		{
			this->scope = this->scope->parent;
		}
		
		llvm_module_t* encode(struct ast_module_t* m)
		{
			this->module = new llvm_module_t("eokas", context);
			this->scope = this->module->root;
			
			if(!this->encode_module(m))
			{
				_DeletePointer(this->module);
				return nullptr;
			}
			return this->module;
		}
		
		bool encode_module(struct ast_module_t* node)
		{
			if(node == nullptr)
				return false;
			
			llvm_type_t* mainRet = module->type_i32;
			std::vector<llvm::Type*> mainArgs;
			llvm::Function* mainPtr = llvm_model_t::declare_func(module->module, "main", mainRet->handle, mainArgs, false);
			this->func = dynamic_cast<llvm_func_t*>(module->new_expr(mainPtr));
			
			this->pushScope(mainPtr);
			{
				llvm::BasicBlock* entry = llvm::BasicBlock::Create(context, "entry", mainPtr);
				builder.SetInsertPoint(entry);
				
				for (auto& stmt: node->get_func()->body)
				{
					if(!this->encode_stmt(stmt))
						return false;
				}
				
				auto& lastOp = builder.GetInsertBlock()->back();
				if(!lastOp.isTerminator())
				{
					if(mainPtr->getReturnType()->isVoidTy())
						builder.CreateRetVoid();
					else
						builder.CreateRet(mainRet->defval);
				}
			}
			this->popScope();
			
			return true;
		}
		
		llvm_type_t* encode_type(struct ast_type_t* node)
		{
			if(node == nullptr)
				return nullptr;
			
			switch (node->category)
			{
				case ast_node_category_t::type_ref:
					return this->encode_type_ref(dynamic_cast<ast_type_ref_t*>(node));
				case ast_node_category_t::type_array:
					return this->encode_type_array(dynamic_cast<ast_type_array_t*>(node));
				default:
					return nullptr;
			}
			
			return nullptr;
		}
		
		llvm_type_t* encode_type_ref(struct ast_type_ref_t* node)
		{
			if(node == nullptr)
			{
				printf("Type node is null. \n");
				return nullptr;
			}
			
			const String& name = node->name;
			auto* type = this->scope->getType(name, true);
			return type;
		}
		
		llvm_type_t* encode_type_array(struct ast_type_array_t* node)
		{
			if(node == nullptr)
			{
				printf("Type node is null. \n");
				return nullptr;
			}
			
			auto* elementType = this->encode_type(node->elementType);
			if(elementType == nullptr)
				return nullptr;
			
			String name = String::format("Array<%s, %d>", elementType->name.cstr(), node->length);
			llvm::Type* handle = llvm::ArrayType::get(elementType->handle, node->length);
			llvm::Value* defval = llvm::ConstantPointerNull::get(handle->getPointerTo());
			return module->new_type(name, handle, defval);
		}
		
		llvm_expr_t* encode_expr(struct ast_expr_t* node)
		{
			if(node == nullptr)
				return nullptr;
			
			switch (node->category)
			{
				case ast_node_category_t::expr_trinary:
					return this->encode_expr_trinary(dynamic_cast<ast_expr_trinary_t*>(node));
				case ast_node_category_t::expr_binary_type:
					return this->encode_expr_binary_type(dynamic_cast<ast_expr_binary_type_t*>(node));
				case ast_node_category_t::expr_binary_value:
					return this->encode_expr_binary_value(dynamic_cast<ast_expr_binary_value_t*>(node));
				case ast_node_category_t::expr_unary:
					return this->encode_expr_unary(dynamic_cast<ast_expr_unary_t*>(node));
				case ast_node_category_t::expr_int:
					return this->encode_expr_int(dynamic_cast<ast_expr_int_t*>(node));
				case ast_node_category_t::expr_float:
					return this->encode_expr_float(dynamic_cast<ast_expr_float_t*>(node));
				case ast_node_category_t::expr_bool:
					return this->encode_expr_bool(dynamic_cast<ast_expr_bool_t*>(node));
				case ast_node_category_t::expr_string:
					return this->encode_expr_string(dynamic_cast<ast_expr_string_t*>(node));
				case ast_node_category_t::expr_symbol_ref:
					return this->encode_expr_symbol_ref(dynamic_cast<ast_expr_symbol_ref_t*>(node));
				case ast_node_category_t::expr_func_def:
					return this->encode_expr_func_def(dynamic_cast<ast_expr_func_def_t*>(node));
				case ast_node_category_t::expr_func_ref:
					return this->encode_expr_func_ref(dynamic_cast<ast_expr_func_ref_t*>(node));
				case ast_node_category_t::expr_array_def:
					return this->encode_expr_array_def(dynamic_cast<ast_expr_array_def_t*>(node));
				case ast_node_category_t::expr_index_ref:
					return this->encode_expr_index_ref(dynamic_cast<ast_expr_index_ref_t*>(node));
				case ast_node_category_t::expr_object_def:
					return this->encode_expr_object_def(dynamic_cast<ast_expr_object_def_t*>(node));
				case ast_node_category_t::expr_object_ref:
					return this->encode_expr_object_ref(dynamic_cast<ast_expr_object_ref_t*>(node));
				default:
					return nullptr;
			}
			return nullptr;
		}
		
		llvm_expr_t* encode_expr_trinary(struct ast_expr_trinary_t* node)
		{
			if(node == nullptr)
				return nullptr;
			
			
			llvm::BasicBlock* trinary_begin = llvm::BasicBlock::Create(context, "trinary.begin", this->scope->func);
			llvm::BasicBlock* trinary_true = llvm::BasicBlock::Create(context, "trinary.true", this->scope->func);
			llvm::BasicBlock* trinary_false = llvm::BasicBlock::Create(context, "trinary.false", this->scope->func);
			llvm::BasicBlock* trinary_end = llvm::BasicBlock::Create(context, "trinary.end", this->scope->func);
			
			builder.CreateBr(trinary_begin);
			builder.SetInsertPoint(trinary_begin);
			llvm_expr_t* condE = this->encode_expr(node->cond);
			if(condE == nullptr)
				return nullptr;
			if(!condE->type->isIntegerTy(1))
			{
				printf("Condition must be a bool value.\n");
				return nullptr;
			}
			auto condV = llvm_model_t::get_value(builder, condE->value);
			builder.CreateCondBr(condV, trinary_true, trinary_false);
			
			builder.SetInsertPoint(trinary_true);
			llvm_expr_t* trueE = this->encode_expr(node->branch_true);
			if(trueE == nullptr)
				return nullptr;
			builder.CreateBr(trinary_end);
			
			builder.SetInsertPoint(trinary_false);
			llvm_expr_t* falseE = this->encode_expr(node->branch_false);
			if(falseE == nullptr)
				return nullptr;
			builder.CreateBr(trinary_end);
			
			builder.SetInsertPoint(trinary_end);
			if(trueE->type != falseE->type)
			{
				printf("Type of true-branch must be the same as false-branch.\n");
				return nullptr;
			}
			
			llvm::PHINode* phi = builder.CreatePHI(trueE->type, 2);
			phi->addIncoming(trueE->value, trinary_true);
			phi->addIncoming(falseE->value, trinary_false);
			
			return module->new_expr(phi);
		}
		
		llvm_expr_t* encode_expr_binary_type(struct ast_expr_binary_type_t* node)
		{
			if(node == nullptr)
				return nullptr;
			
			auto left = this->encode_expr(node->left);
			auto right = this->encode_type(node->right);
			if(left == nullptr || right == nullptr)
				return nullptr;
			
			auto lhs = llvm_model_t::get_value(builder, left->value);
			
			switch (node->op)
			{
				case ast_binary_oper_t::Is:
				{
					if(right->handle == lhs->getType())
						return module->new_expr(builder.getTrue());
					else
						return module->new_expr(builder.getFalse());
				}
					break;
				case ast_binary_oper_t::As:
				{
					if(lhs->getType()->isSingleValueType())
					{
						if(right->handle == module->type_string_ref->handle)
						{
							llvm::Value* value = module->value_to_string(this->scope->func, builder, lhs);
							return module->new_expr(value);
						}
						return nullptr;
					}
					else
					{
						printf("Complex cast is not supported.\n");
						return nullptr;
					}
				}
					break;
				default:
					return nullptr;
			}
			
			return nullptr;
		}
		
		llvm_expr_t* encode_expr_binary_value(struct ast_expr_binary_value_t* node)
		{
			if(node == nullptr)
				return nullptr;
			
			auto left = this->encode_expr(node->left);
			auto right = this->encode_expr(node->right);
			if(left == nullptr || right == nullptr)
				return nullptr;
			
			auto lhs = llvm_model_t::get_value(builder, left->value);
			auto rhs = llvm_model_t::get_value(builder, right->value);
			
			switch (node->op)
			{
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
					return this->encode_expr_binary_bitshl(lhs, rhs);
				case ast_binary_oper_t::ShiftR:
					return this->encode_expr_binary_bitshr(lhs, rhs);
				default:
					return nullptr;
			}
		}
		
		llvm_expr_t* encode_expr_binary_or(llvm::Value* lhs, llvm::Value* rhs)
		{
			auto ltype = lhs->getType();
			auto rtype = rhs->getType();
			
			if((!ltype->isIntegerTy(1)) || (!rtype->isIntegerTy(1)))
			{
				printf("LHS or RHS is not bool value. \n");
				return nullptr;
			}
			
			return module->new_expr(builder.CreateOr(lhs, rhs));
		}
		
		llvm_expr_t* encode_expr_binary_and(llvm::Value* lhs, llvm::Value* rhs)
		{
			auto ltype = lhs->getType();
			auto rtype = rhs->getType();
			
			if((!ltype->isIntegerTy(1)) || (!rtype->isIntegerTy(1)))
			{
				printf("LHS or RHS is not bool value. \n");
				return nullptr;
			}
			
			return module->new_expr(builder.CreateAnd(lhs, rhs));
		}
		
		llvm_expr_t* encode_expr_binary_eq(llvm::Value* lhs, llvm::Value* rhs)
		{
			auto ltype = lhs->getType();
			auto rtype = rhs->getType();
			
			if(ltype->isIntegerTy() && rtype->isIntegerTy())
				return module->new_expr(builder.CreateICmpEQ(lhs, rhs));
			
			if(ltype->isFloatingPointTy() && rtype->isFloatingPointTy())
				return module->new_expr(builder.CreateFCmpOEQ(lhs, rhs));
			
			if(ltype->isPointerTy() && rtype->isPointerTy())
			{
				return module->new_expr(builder.CreateICmpEQ(builder.CreatePtrToInt(lhs, llvm::Type::getInt64Ty(context)), builder.CreatePtrToInt(rhs, llvm::Type::getInt64Ty(context))));
			}
			
			if(ltype->isIntegerTy() && rtype->isFloatingPointTy())
			{
				return module->new_expr(builder.CreateFCmpOEQ(builder.CreateSIToFP(lhs, llvm::Type::getDoubleTy(context)), rhs));
			}
			
			if(ltype->isFloatingPointTy() && rtype->isIntegerTy())
			{
				return module->new_expr(builder.CreateFCmpOEQ(lhs, builder.CreateSIToFP(rhs, llvm::Type::getDoubleTy(context))));
			}
			
			printf("Type of LHS or RHS is invalid.\n");
			return nullptr;
		}
		
		llvm_expr_t* encode_expr_binary_ne(llvm::Value* lhs, llvm::Value* rhs)
		{
			auto ltype = lhs->getType();
			auto rtype = rhs->getType();
			
			if(ltype->isIntegerTy() && rtype->isIntegerTy())
				return module->new_expr(builder.CreateICmpNE(lhs, rhs));
			
			if(ltype->isFloatingPointTy() && rtype->isFloatingPointTy())
				return module->new_expr(builder.CreateFCmpONE(lhs, rhs));
			
			if(ltype->isPointerTy() && rtype->isPointerTy())
			{
				return module->new_expr(builder.CreateICmpNE(builder.CreatePtrToInt(lhs, llvm::Type::getInt64Ty(context)), builder.CreatePtrToInt(rhs, llvm::Type::getInt64Ty(context))));
			}
			
			if(ltype->isIntegerTy() && rtype->isFloatingPointTy())
			{
				return module->new_expr(builder.CreateFCmpONE(builder.CreateSIToFP(lhs, llvm::Type::getDoubleTy(context)), rhs));
			}
			
			if(ltype->isFloatingPointTy() && rtype->isIntegerTy())
			{
				return module->new_expr(builder.CreateFCmpONE(lhs, builder.CreateSIToFP(rhs, llvm::Type::getDoubleTy(context))));
			}
			
			printf("Type of LHS or RHS is invalid.\n");
			return nullptr;
		}
		
		llvm_expr_t* encode_expr_binary_le(llvm::Value* lhs, llvm::Value* rhs)
		{
			auto ltype = lhs->getType();
			auto rtype = rhs->getType();
			
			if(ltype->isIntegerTy() && rtype->isIntegerTy())
				return module->new_expr(builder.CreateICmpSLE(lhs, rhs));
			
			if(ltype->isFloatingPointTy() && rtype->isFloatingPointTy())
				return module->new_expr(builder.CreateFCmpOLE(lhs, rhs));
			
			if(ltype->isPointerTy() && rtype->isPointerTy())
			{
				return module->new_expr(builder.CreateICmpULE(builder.CreatePtrToInt(lhs, llvm::Type::getInt64Ty(context)), builder.CreatePtrToInt(rhs, llvm::Type::getInt64Ty(context))));
			}
			
			if(ltype->isIntegerTy() && rtype->isFloatingPointTy())
			{
				return module->new_expr(builder.CreateFCmpOLE(builder.CreateSIToFP(lhs, llvm::Type::getDoubleTy(context)), rhs));
			}
			
			if(ltype->isFloatingPointTy() && rtype->isIntegerTy())
			{
				return module->new_expr(builder.CreateFCmpOLE(lhs, builder.CreateSIToFP(rhs, llvm::Type::getDoubleTy(context))));
			}
			
			printf("Type of LHS or RHS is invalid.\n");
			return nullptr;
		}
		
		llvm_expr_t* encode_expr_binary_ge(llvm::Value* lhs, llvm::Value* rhs)
		{
			auto ltype = lhs->getType();
			auto rtype = rhs->getType();
			
			if(ltype->isIntegerTy() && rtype->isIntegerTy())
				return module->new_expr(builder.CreateICmpSGE(lhs, rhs));
			
			if(ltype->isFloatingPointTy() && rtype->isFloatingPointTy())
				return module->new_expr(builder.CreateFCmpOGE(lhs, rhs));
			
			if(ltype->isPointerTy() && rtype->isPointerTy())
			{
				return module->new_expr(builder.CreateICmpUGE(builder.CreatePtrToInt(lhs, llvm::Type::getInt64Ty(context)), builder.CreatePtrToInt(rhs, llvm::Type::getInt64Ty(context))));
			}
			
			if(ltype->isIntegerTy() && rtype->isFloatingPointTy())
			{
				return module->new_expr(builder.CreateFCmpOGE(builder.CreateSIToFP(lhs, llvm::Type::getDoubleTy(context)), rhs));
			}
			
			if(ltype->isFloatingPointTy() && rtype->isIntegerTy())
			{
				return module->new_expr(builder.CreateFCmpOGE(lhs, builder.CreateSIToFP(rhs, llvm::Type::getDoubleTy(context))));
			}
			
			printf("Type of LHS or RHS is invalid.\n");
			return nullptr;
		}
		
		llvm_expr_t* encode_expr_binary_lt(llvm::Value* lhs, llvm::Value* rhs)
		{
			auto ltype = lhs->getType();
			auto rtype = rhs->getType();
			
			if(ltype->isIntegerTy() && rtype->isIntegerTy())
				return module->new_expr(builder.CreateICmpSLT(lhs, rhs));
			
			if(ltype->isFloatingPointTy() && rtype->isFloatingPointTy())
				return module->new_expr(builder.CreateFCmpOLT(lhs, rhs));
			
			if(ltype->isPointerTy() && rtype->isPointerTy())
			{
				return module->new_expr(builder.CreateICmpULT(builder.CreatePtrToInt(lhs, llvm::Type::getInt64Ty(context)), builder.CreatePtrToInt(rhs, llvm::Type::getInt64Ty(context))));
			}
			
			if(ltype->isIntegerTy() && rtype->isFloatingPointTy())
			{
				return module->new_expr(builder.CreateFCmpOLT(builder.CreateSIToFP(lhs, llvm::Type::getDoubleTy(context)), rhs));
			}
			
			if(ltype->isFloatingPointTy() && rtype->isIntegerTy())
			{
				return module->new_expr(builder.CreateFCmpOLT(lhs, builder.CreateSIToFP(rhs, llvm::Type::getDoubleTy(context))));
			}
			
			printf("Type of LHS or RHS is invalid.\n");
			return nullptr;
		}
		
		llvm_expr_t* encode_expr_binary_gt(llvm::Value* lhs, llvm::Value* rhs)
		{
			auto ltype = lhs->getType();
			auto rtype = rhs->getType();
			
			if(ltype->isIntegerTy() && rtype->isIntegerTy())
				return module->new_expr(builder.CreateICmpSGT(lhs, rhs));
			
			if(ltype->isFloatingPointTy() && rtype->isFloatingPointTy())
				return module->new_expr(builder.CreateFCmpOGT(lhs, rhs));
			
			if(ltype->isPointerTy() && rtype->isPointerTy())
			{
				return module->new_expr(builder.CreateICmpUGT(builder.CreatePtrToInt(lhs, llvm::Type::getInt64Ty(context)), builder.CreatePtrToInt(rhs, llvm::Type::getInt64Ty(context))));
			}
			
			if(ltype->isIntegerTy() && rtype->isFloatingPointTy())
			{
				return module->new_expr(builder.CreateFCmpOGT(builder.CreateSIToFP(lhs, llvm::Type::getDoubleTy(context)), rhs));
			}
			
			if(ltype->isFloatingPointTy() && rtype->isIntegerTy())
			{
				return module->new_expr(builder.CreateFCmpOGT(lhs, builder.CreateSIToFP(rhs, llvm::Type::getDoubleTy(context))));
			}
			
			printf("Type of LHS or RHS is invalid.\n");
			return nullptr;
		}
		
		llvm_expr_t* encode_expr_binary_add(llvm::Value* lhs, llvm::Value* rhs)
		{
			auto ltype = lhs->getType();
			auto rtype = rhs->getType();
			
			if(ltype->isIntegerTy() && rtype->isIntegerTy())
				return module->new_expr(builder.CreateAdd(lhs, rhs));
			
			if(ltype->isFloatingPointTy() && rtype->isFloatingPointTy())
				return module->new_expr(builder.CreateFAdd(lhs, rhs));
			
			if(ltype->isIntegerTy() && rtype->isFloatingPointTy())
			{
				return module->new_expr(builder.CreateFAdd(builder.CreateSIToFP(lhs, llvm::Type::getDoubleTy(context)), rhs));
			}
			
			if(ltype->isFloatingPointTy() && rtype->isIntegerTy())
			{
				return module->new_expr(builder.CreateFAdd(lhs, builder.CreateSIToFP(rhs, llvm::Type::getDoubleTy(context))));
			}
			
			printf("Type of LHS or RHS is invalid.\n");
			return nullptr;
		}
		
		llvm_expr_t* encode_expr_binary_sub(llvm::Value* lhs, llvm::Value* rhs)
		{
			auto ltype = lhs->getType();
			auto rtype = rhs->getType();
			
			if(ltype->isIntegerTy() && rtype->isIntegerTy())
				return module->new_expr(builder.CreateSub(lhs, rhs));
			
			if(ltype->isFloatingPointTy() && rtype->isFloatingPointTy())
				return module->new_expr(builder.CreateFSub(lhs, rhs));
			
			if(ltype->isIntegerTy() && rtype->isFloatingPointTy())
			{
				return module->new_expr(builder.CreateFSub(builder.CreateSIToFP(lhs, llvm::Type::getDoubleTy(context)), rhs));
			}
			
			if(ltype->isFloatingPointTy() && rtype->isIntegerTy())
			{
				return module->new_expr(builder.CreateFSub(lhs, builder.CreateSIToFP(rhs, llvm::Type::getDoubleTy(context))));
			}
			
			printf("Type of LHS or RHS is invalid.\n");
			return nullptr;
		}
		
		llvm_expr_t* encode_expr_binary_mul(llvm::Value* lhs, llvm::Value* rhs)
		{
			auto ltype = lhs->getType();
			auto rtype = rhs->getType();
			
			if(ltype->isIntegerTy() && rtype->isIntegerTy())
				return module->new_expr(builder.CreateMul(lhs, rhs));
			
			if(ltype->isFloatingPointTy() && rtype->isFloatingPointTy())
				return module->new_expr(builder.CreateFMul(lhs, rhs));
			
			if(ltype->isIntegerTy() && rtype->isFloatingPointTy())
			{
				return module->new_expr(builder.CreateFMul(builder.CreateSIToFP(lhs, llvm::Type::getDoubleTy(context)), rhs));
			}
			
			if(ltype->isFloatingPointTy() && rtype->isIntegerTy())
			{
				return module->new_expr(builder.CreateFMul(lhs, builder.CreateSIToFP(rhs, llvm::Type::getDoubleTy(context))));
			}
			
			printf("Type of LHS or RHS is invalid.\n");
			return nullptr;
		}
		
		llvm_expr_t* encode_expr_binary_div(llvm::Value* lhs, llvm::Value* rhs)
		{
			auto ltype = lhs->getType();
			auto rtype = rhs->getType();
			
			if(ltype->isIntegerTy() && rtype->isIntegerTy())
				return module->new_expr(builder.CreateSDiv(lhs, rhs));
			
			if(ltype->isFloatingPointTy() && rtype->isFloatingPointTy())
				return module->new_expr(builder.CreateFMul(lhs, rhs));
			
			if(ltype->isIntegerTy() && rtype->isFloatingPointTy())
			{
				return module->new_expr(builder.CreateFDiv(builder.CreateSIToFP(lhs, llvm::Type::getDoubleTy(context)), rhs));
			}
			
			if(ltype->isFloatingPointTy() && rtype->isIntegerTy())
			{
				return module->new_expr(builder.CreateFDiv(lhs, builder.CreateSIToFP(rhs, llvm::Type::getDoubleTy(context))));
			}
			
			printf("Type of LHS or RHS is invalid.\n");
			return nullptr;
		}
		
		llvm_expr_t* encode_expr_binary_mod(llvm::Value* lhs, llvm::Value* rhs)
		{
			auto ltype = lhs->getType();
			auto rtype = rhs->getType();
			
			if(ltype->isIntegerTy() && rtype->isIntegerTy())
				return module->new_expr(builder.CreateSRem(lhs, rhs));
			
			if(ltype->isFloatingPointTy() && rtype->isFloatingPointTy())
				return module->new_expr(builder.CreateFRem(lhs, rhs));
			
			if(ltype->isIntegerTy() && rtype->isFloatingPointTy())
			{
				return module->new_expr(builder.CreateFRem(builder.CreateSIToFP(lhs, llvm::Type::getDoubleTy(context)), rhs));
			}
			
			if(ltype->isFloatingPointTy() && rtype->isIntegerTy())
			{
				return module->new_expr(builder.CreateFRem(lhs, builder.CreateSIToFP(rhs, llvm::Type::getDoubleTy(context))));
			}
			
			printf("Type of LHS or RHS is invalid.\n");
			return nullptr;
		}
		
		llvm_expr_t* encode_expr_binary_bitand(llvm::Value* lhs, llvm::Value* rhs)
		{
			auto ltype = lhs->getType();
			auto rtype = rhs->getType();
			
			if(ltype->isIntegerTy() && rtype->isIntegerTy())
				return module->new_expr(builder.CreateAnd(lhs, rhs));
			
			printf("Type of LHS or RHS is invalid.\n");
			return nullptr;
		}
		
		llvm_expr_t* encode_expr_binary_bitor(llvm::Value* lhs, llvm::Value* rhs)
		{
			auto ltype = lhs->getType();
			auto rtype = rhs->getType();
			
			if(ltype->isIntegerTy() && rtype->isIntegerTy())
				return module->new_expr(builder.CreateOr(lhs, rhs));
			
			printf("Type of LHS or RHS is invalid.\n");
			return nullptr;
		}
		
		llvm_expr_t* encode_expr_binary_bitxor(llvm::Value* lhs, llvm::Value* rhs)
		{
			auto ltype = lhs->getType();
			auto rtype = rhs->getType();
			
			if(ltype->isIntegerTy() && rtype->isIntegerTy())
				return module->new_expr(builder.CreateXor(lhs, rhs));
			
			printf("Type of LHS or RHS is invalid.\n");
			return nullptr;
		}
		
		llvm_expr_t* encode_expr_binary_bitshl(llvm::Value* lhs, llvm::Value* rhs)
		{
			auto ltype = lhs->getType();
			auto rtype = rhs->getType();
			
			if(ltype->isIntegerTy() && rtype->isIntegerTy())
				return module->new_expr(builder.CreateShl(lhs, rhs));
			
			printf("Type of LHS or RHS is invalid.\n");
			return nullptr;
		}
		
		llvm_expr_t* encode_expr_binary_bitshr(llvm::Value* lhs, llvm::Value* rhs)
		{
			auto ltype = lhs->getType();
			auto rtype = rhs->getType();
			
			if(ltype->isIntegerTy() && rtype->isIntegerTy())
			{
				// 逻辑右移：在左边补 0
				// 算术右移：在左边补 符号位
				// 我们采用逻辑右移
				return module->new_expr(builder.CreateLShr(lhs, rhs));
			}
			
			printf("Type of LHS or RHS is invalid.\n");
			return nullptr;
		}
		
		llvm_expr_t* encode_expr_unary(struct ast_expr_unary_t* node)
		{
			if(node == nullptr)
				return nullptr;
			
			auto right = this->encode_expr(node->right);
			if(right == nullptr)
				return nullptr;
			
			auto rhs = llvm_model_t::get_value(builder, right->value);
			
			switch (node->op)
			{
				case ast_unary_oper_t::Pos:
					return module->new_expr(rhs);
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
		
		llvm_expr_t* encode_expr_unary_neg(llvm::Value* rhs)
		{
			auto rtype = rhs->getType();
			
			if(rtype->isIntegerTy())
				return module->new_expr(builder.CreateNeg(rhs));
			
			if(rtype->isFloatingPointTy())
				return module->new_expr(builder.CreateFNeg(rhs));
			
			printf("Type of RHS is invalid.\n");
			return nullptr;
		}
		
		llvm_expr_t* encode_expr_unary_not(llvm::Value* rhs)
		{
			auto rtype = rhs->getType();
			
			if(rtype->isIntegerTy() && rtype->getIntegerBitWidth() == 1)
				return module->new_expr(builder.CreateNot(rhs));
			
			printf("Type of RHS is invalid.\n");
			return nullptr;
		}
		
		llvm_expr_t* encode_expr_unary_flip(llvm::Value* rhs)
		{
			auto rtype = rhs->getType();
			
			if(rtype->isIntegerTy())
			{
				return module->new_expr(builder.CreateXor(rhs, llvm::ConstantInt::get(rtype, llvm::APInt(rtype->getIntegerBitWidth(), 0xFFFFFFFF))));
			}
			printf("Type of RHS is invalid.\n");
			return nullptr;
		}
		
		llvm_expr_t* encode_expr_int(struct ast_expr_int_t* node)
		{
			if(node == nullptr)
				return nullptr;
			
			u64_t vals = *((u64_t*) &(node->value));
			u32_t bits = vals>0xFFFFFFFF ? 64 : 32;
			
			return module->new_expr(llvm::ConstantInt::get(context, llvm::APInt(bits, node->value)));
		}
		
		llvm_expr_t* encode_expr_float(struct ast_expr_float_t* node)
		{
			if(node == nullptr)
				return nullptr;
			
			return module->new_expr(llvm::ConstantFP::get(context, llvm::APFloat(node->value)));
		}
		
		llvm_expr_t* encode_expr_bool(struct ast_expr_bool_t* node)
		{
			if(node == nullptr)
				return nullptr;
			return module->new_expr(llvm::ConstantInt::getBool(context, node->value));
		}
		
		llvm_expr_t* encode_expr_string(struct ast_expr_string_t* node)
		{
			if(node == nullptr)
				return nullptr;
			
			auto str = module->make_string(this->scope->func, builder, node->value.cstr());
			
			return module->new_expr(str);
		}
		
		llvm_expr_t* encode_expr_symbol_ref(struct ast_expr_symbol_ref_t* node)
		{
			if(node == nullptr)
				return nullptr;
			
			auto symbol = this->scope->getSymbol(node->name, true);
			if(symbol == nullptr)
			{
				printf("Symbol '%s' is undefined.\n", node->name.cstr());
				return nullptr;
			}
			
			// up-value
			if(symbol->scope->func != this->func->value)
			{
				this->func->upvals.add(node->name, symbol);
			}
			
			return symbol;
		}
		
		llvm_expr_t* encode_expr_func_def(struct ast_expr_func_def_t* node)
		{
			if(node == nullptr)
				return nullptr;
			
			auto* retType = this->encode_type(node->type);
			if(retType == nullptr)
				return nullptr;
			
			std::vector<llvm::Type*> argTypes;
			for (auto arg: node->args)
			{
				auto* argType = this->encode_type(arg->type);
				if(argType == nullptr)
					return nullptr;
				if(argType->handle->isFunctionTy() || argType->handle->isStructTy() || argType->handle->isArrayTy())
					argTypes.push_back(argType->handle->getPointerTo());
				else
					argTypes.push_back(argType->handle);
			}
			
			llvm::FunctionType* funcType = llvm::FunctionType::get(retType->handle, argTypes, false);
			llvm::Function* funcPtr = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, "", module->module);
			
			u32_t index = 0;
			for (auto& arg: funcPtr->args())
			{
				const char* name = node->args[index++]->name.cstr();
				arg.setName(name);
			}
			
			auto oldFunc = this->func;
			auto oldIB = builder.GetInsertBlock();
			
			this->func = dynamic_cast<llvm_func_t*>(module->new_expr(funcPtr));
			
			this->pushScope(funcPtr);
			{
				llvm::BasicBlock* entry = llvm::BasicBlock::Create(context, "entry", funcPtr);
				builder.SetInsertPoint(entry);
				
				// self
				auto self = module->new_expr(funcPtr);
				this->scope->addSymbol("self", self);
				
				// args
				for (auto& arg: funcPtr->args())
				{
					const char* name = arg.getName().data();
					auto expr = module->new_expr(&arg);
					if(!this->scope->addSymbol(name, expr))
					{
						printf("The symbol name '%s' is already existed.\n", name);
						return nullptr;
					}
				}
				
				// body
				llvm::BasicBlock* body = llvm::BasicBlock::Create(context, "body", funcPtr);
				builder.SetInsertPoint(body);
				for (auto& stmt: node->body)
				{
					if(!this->encode_stmt(stmt))
						return nullptr;
				}
				auto& lastOp = builder.GetInsertBlock()->back();
				if(!lastOp.isTerminator())
				{
					if(funcPtr->getReturnType()->isVoidTy())
						builder.CreateRetVoid();
					else
						builder.CreateRet(retType->defval);
				}
				
				// up-value
				builder.SetInsertPoint(entry);
				for (auto& upval: this->func->upvals.table)
				{
					const String& name = upval.first;
					llvm_expr_t* expr = upval.second;
					
					llvm::Value* val = llvm_model_t::get_value(builder, expr->value);
					llvm::Value* ptr = builder.CreateAlloca(val->getType());
					builder.CreateStore(val, ptr);
					if(!this->scope->addSymbol(name, module->new_expr(val)))
					{
						printf("The symbol name '%s' is already existed.\n", name.cstr());
						return nullptr;
					}
				}
				builder.CreateBr(body);
			}
			this->popScope();
			
			this->func = oldFunc;
			builder.SetInsertPoint(oldIB);
			
			return module->new_expr(funcPtr);
		}
		
		llvm_expr_t* encode_expr_func_ref(struct ast_expr_func_ref_t* node)
		{
			if(node == nullptr)
				return nullptr;
			
			llvm_expr_t* funcExpr = this->encode_expr(node->func);
			if(funcExpr == nullptr)
			{
				printf("Function is undefined.\n");
				return nullptr;
			}
			
			auto funcPtr = llvm_model_t::get_value(builder, funcExpr->value);
			llvm::FunctionType* funcType = nullptr;
			if(funcExpr->type->isFunctionTy())
			{
				funcType = llvm::cast<llvm::FunctionType>(funcExpr->type);
			}
			else if(funcExpr->type->isPointerTy() && funcExpr->type->getPointerElementType()->isFunctionTy())
			{
				funcType = llvm::cast<llvm::FunctionType>(funcExpr->type->getPointerElementType());
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
				auto* argE = this->encode_expr(node->args.at(i));
				if(argE == nullptr)
					return nullptr;
				
				llvm::Value* argV = argE->value;
				if(argE->is_symbol())
				{
					argV = builder.CreateLoad(argE->value);
				}
				argV = llvm_model_t::get_value(builder, argV);
				
				if(paramT != argV->getType())
				{
					if(paramT == module->type_i8_ref->handle)
					{
						argV = module->value_to_cstr(scope->func, builder, argV);
					}
					if(paramT == module->type_string_ref->handle)
					{
						argV = module->value_to_string(scope->func, builder, argV);
					}
					else if(!argV->getType()->canLosslesslyBitCastTo(paramT))
					{
						printf("The type of param[%d] can't cast to the param type of function.\n", i);
						return nullptr;
					}
				}
				
				args.push_back(argV);
			}
			
			llvm::Value* retval = builder.CreateCall(funcType, funcPtr, args);
			return module->new_expr(retval);
		}
		
		llvm_expr_t* encode_expr_array_def(struct ast_expr_array_def_t* node)
		{
			if(node == nullptr)
				return nullptr;
			
			std::vector<llvm::Value*> arrayElements;
			llvm::Type* arrayElementType = nullptr;
			for (auto element: node->elements)
			{
				auto elementE = this->encode_expr(element);
				if(elementE == nullptr)
					return nullptr;
				
				auto elementV = elementE->value;
				if(elementE->is_symbol())
				{
					elementV = builder.CreateLoad(elementE->value);
				}
				
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
			
			auto arrayType = llvm::ArrayType::get(arrayElementType, node->elements.size());
			auto arrayPtr = module->make(this->scope->func, builder, arrayType);
			for (u32_t i = 0; i<arrayElements.size(); i++)
			{
				auto elementV = arrayElements.at(i);
				auto elementP = builder.CreateConstGEP2_32(arrayType, arrayPtr, 0, i);
				builder.CreateStore(elementV, elementP);
			}
			
			return module->new_expr(arrayPtr);
		}
		
		llvm_expr_t* encode_expr_index_ref(struct ast_expr_index_ref_t* node)
		{
			if(node == nullptr)
				return nullptr;
			
			auto objE = this->encode_expr(node->obj);
			auto keyE = this->encode_expr(node->key);
			if(objE == nullptr || keyE == nullptr)
				return nullptr;
			
			auto objV = llvm_model_t::get_value(builder, objE->value);
			auto keyV = llvm_model_t::get_value(builder, keyE->value);
			if(!objV->getType()->isPointerTy() || !objV->getType()->getPointerElementType()->isArrayTy())
			{
				printf("Index-Access is not defined on the object.\n");
				return nullptr;
			}
			if(!keyV->getType()->isIntegerTy())
			{
				printf("The type of index is invalid.\n");
				return nullptr;
			}
			
			llvm::Value* ptr = builder.CreateGEP(objV, {builder.getInt32(0), keyV});
			return module->new_expr(ptr);
		}
		
		llvm_expr_t* encode_expr_object_def(struct ast_expr_object_def_t* node)
		{
			if(node == nullptr)
				return nullptr;
			
			auto* structType = this->encode_type(node->type);
			if(structType == nullptr)
				return nullptr;
			
			for (auto& objectMember: node->members)
			{
				auto structMember = structType->get_member(objectMember.first);
				if(structMember == nullptr)
				{
					printf("Object member '%s' is not defined in struct.\n", objectMember.first.cstr());
					return nullptr;
				}
				// todo: check object-member-type equals to struct-member-type.
			}
			
			// make object instance
			llvm::Value* instance = module->make(this->scope->func, builder, structType->handle);
			
			for (u32_t index = 0; index<structType->members.size(); index++)
			{
				auto& structMember = structType->members.at(index);
				auto objectMember = node->members.find(structMember->name);
				llvm::Value* memV = nullptr;
				if(objectMember != node->members.end())
				{
					auto memE = this->encode_expr(objectMember->second);
					if(memE == nullptr)
						return nullptr;
					memV = llvm_model_t::get_value(builder, memE->value);
				}
				else
				{
					memV = structMember->value != nullptr ? structMember->value->value : structMember->type->defval;
				}
				
				if(memV)
				{
					String memN = String::format("this.%s", structMember->name.cstr());
					llvm::Value* memP = builder.CreateStructGEP(structType->handle, instance, index, memN.cstr());
					builder.CreateStore(memV, memP);
				}
			}
			
			return module->new_expr(instance);
		}
		
		llvm_expr_t* encode_expr_object_ref(struct ast_expr_object_ref_t* node)
		{
			if(node == nullptr)
				return nullptr;
			
			auto objectE = this->encode_expr(node->obj);
			
			// Maybe a static-member
			if(objectE == nullptr && node->obj->category == ast_node_category_t::expr_symbol_ref)
			{
				auto typeSymbol = dynamic_cast<ast_expr_symbol_ref_t*>(node->obj)->name;
			}
			
			auto key = builder.CreateGlobalString(node->key.cstr());
			if(objectE == nullptr || key == nullptr)
				return nullptr;
			
			auto objectV = llvm_model_t::get_value(builder, objectE->value);
			auto objectT = objectV->getType();
			if(!(objectT->isPointerTy() && objectT->getPointerElementType()->isStructTy()))
			{
				printf("The value is not a object reference.\n");
				return nullptr;
			}
			
			auto structTypeDef = module->get_type(objectT->getPointerElementType());
			if(structTypeDef == nullptr)
			{
				printf("Can't find the type of this value.\n");
				return nullptr;
			}
			
			
			auto index = structTypeDef->get_member_index(node->key);
			if(index == -1)
			{
				printf("The object doesn't have a member named '%s'. \n", node->key.cstr());
				return nullptr;
			}
			
			auto structType = structTypeDef->handle;
			llvm::Value* value = builder.CreateStructGEP(structType, objectV, index);
			return module->new_expr(value);
		}
		
		bool encode_stmt(struct ast_stmt_t* node)
		{
			if(node == nullptr)
				return false;
			
			switch (node->category)
			{
				case ast_node_category_t::stmt_struct_def:
					return this->encode_stmt_struct_def(dynamic_cast<ast_stmt_struct_def_t*>(node));
				case ast_node_category_t::stmt_enum_def:
					return this->encode_stmt_enum_def(dynamic_cast<ast_stmt_enum_def_t*>(node));
				case ast_node_category_t::stmt_proc_def:
					return this->encode_stmt_proc_def(dynamic_cast<ast_stmt_proc_def_t*>(node));
				case ast_node_category_t::stmt_symbol_def:
					return this->encode_stmt_symbol_def(dynamic_cast<ast_stmt_symbol_def_t*>(node));
				case ast_node_category_t::stmt_break:
					return this->encode_stmt_break(dynamic_cast<ast_stmt_break_t*>(node));
				case ast_node_category_t::stmt_continue:
					return this->encode_stmt_continue(dynamic_cast<ast_stmt_continue_t*>(node));
				case ast_node_category_t::stmt_return:
					return this->encode_stmt_return(dynamic_cast<ast_stmt_return_t*>(node));
				case ast_node_category_t::stmt_if:
					return this->encode_stmt_if(dynamic_cast<ast_stmt_if_t*>(node));
				case ast_node_category_t::stmt_loop:
					return this->encode_stmt_loop(dynamic_cast<ast_stmt_loop_t*>(node));
				case ast_node_category_t::stmt_block:
					return this->encode_stmt_block(dynamic_cast<ast_stmt_block_t*>(node));
				case ast_node_category_t::stmt_assign:
					return this->encode_stmt_assign(dynamic_cast<ast_stmt_assign_t*>(node));
				case ast_node_category_t::stmt_call:
					return this->encode_stmt_call(dynamic_cast<ast_stmt_call_t*>(node));
				default:
					return false;
			}
			
			return false;
		}
		
		bool encode_stmt_struct_def(struct ast_stmt_struct_def_t* node)
		{
			if(node == nullptr)
				return false;
			
			const String staticTypePrefix = "$_Static";
			const String staticMemberName = "$_static";
			
			auto* thisStaticType = module->new_type(staticTypePrefix + node->name, nullptr, nullptr);
			auto* thisInstanceType = module->new_type(node->name, nullptr, nullptr);
			
			if(node->base != nullptr)
			{
				auto baseInstanceType = this->encode_type(node->base);
				if(baseInstanceType == nullptr)
					return false;
				auto baseStaticType = this->scope->getType(staticTypePrefix + baseInstanceType->name, true);
				if(baseStaticType == nullptr)
					return false;
				
				String err;
				if(!thisStaticType->extends(baseStaticType, err))
				{
					printf(err.cstr());
					return false;
				}
				if(!thisInstanceType->extends(baseInstanceType, err))
				{
					printf(err.cstr());
					return false;
				}
			}
			
			for (const auto& thisMember: node->members)
			{
				auto& mem = thisMember.second;
				if(mem->isStatic)
				{
					const String& memName = mem->name;
					if(thisStaticType->get_member(memName) != nullptr)
					{
						printf("The member named '%s' is already exists.\n", memName.cstr());
						return false;
					}
					
					const auto& memType = this->encode_type(mem->type);
					if(memType == nullptr)
						return false;
					
					const auto& memValue = this->encode_expr(mem->value);
					if(memValue == nullptr)
						return false;
					
					thisStaticType->add_member(memName, memType, memValue);
				}
				else
				{
					const String& memName = mem->name;
					if(thisInstanceType->get_member(memName) != nullptr)
					{
						printf("The member named '%s' is already exists.\n", memName.cstr());
						return false;
					}
					
					const auto& memType = this->encode_type(mem->type);
					if(memType == nullptr)
						return false;
					
					const auto& memValue = this->encode_expr(mem->value);
					if(memValue == nullptr)
						return false;
					
					thisInstanceType->add_member(memName, memType, memValue);
				}
			}
			
			thisStaticType->resolve();
			thisInstanceType->resolve();
			if(!this->scope->addType(thisStaticType->name, thisStaticType) || !this->scope->addType(thisInstanceType->name, thisInstanceType))
			{
				printf("There is a same type named %s in this scope.\n", thisInstanceType->name.cstr());
				return false;
			}
			
			// make static object
			llvm::Value* staticV = module->make(this->scope->func, builder, thisStaticType->handle);
			llvm_expr_t* staticE = module->new_expr(staticV);
			
			for (u32_t index = 0; index<thisStaticType->members.size(); index++)
			{
				auto& mem = thisStaticType->members.at(index);
				auto& memV = mem->value != nullptr ? mem->value->value : mem->type->defval;
				
				if(memV != nullptr)
				{
					String memN = String::format("%s.%s", thisInstanceType->name.cstr(), mem->name.cstr());
					llvm::Value* memP = builder.CreateStructGEP(thisStaticType->handle, staticV, index, memN.cstr());
					builder.CreateStore(memV, memP);
				}
			}
			
			if(!this->scope->addSymbol(thisInstanceType->name, staticE))
			{
				printf("There is a same symbol named %s in this scope.\n", thisInstanceType->name.cstr());
				return false;
			}
			
			return true;
		}
		
		bool encode_stmt_enum_def(struct ast_stmt_enum_def_t* node)
		{
			if(node == nullptr)
				return false;
			
			const String name = node->name;
			
			auto* thisStaticType = module->new_type(node->name, nullptr, nullptr);
			for (const auto& thisMember: node->members)
			{
				auto memName = thisMember.first;
				if(thisStaticType->get_member(memName) != nullptr)
				{
					printf("The member named '%s' is already exists.\n", memName.cstr());
					return false;
				}
				
				auto v = builder.getInt32(thisMember.second);
				auto o = builder.CreateAlloca(module->type_enum->handle);
				auto n = String::format("%s.%s", node->name.cstr(), memName.cstr());
				auto p = builder.CreateStructGEP(module->type_enum->handle, o, 0, n.cstr());
				builder.CreateStore(v, p);
				auto memValue = module->new_expr(o);
				
				thisStaticType->add_member(memName, module->type_enum, memValue);
			}
			
			thisStaticType->resolve();
			
			// make static object
			llvm::Value* staticV = module->make(this->scope->func, builder, thisStaticType->handle);
			llvm_expr_t* staticE = module->new_expr(staticV);
			
			for (u32_t index = 0; index<thisStaticType->members.size(); index++)
			{
				auto& mem = thisStaticType->members.at(index);
				auto memV = builder.CreateLoad(mem->value->value);
				
				String memN = String::format("%s.%s", name.cstr(), mem->name.cstr());
				llvm::Value* memP = builder.CreateStructGEP(thisStaticType->handle, staticV, index, memN.cstr());
				builder.CreateStore(memV, memP);
			}
			
			if(!this->scope->addSymbol(name, staticE))
			{
				printf("There is a same symbol named %s in this scope.\n", name.cstr());
				return false;
			}
			
			return true;
		}
		
		bool encode_stmt_proc_def(struct ast_stmt_proc_def_t* node)
		{
			if(node == nullptr)
				return false;
			
			if(this->scope->getType(node->name, false) != nullptr)
				return false;
			
			auto* retType = this->encode_type(node->type);
			
			std::vector<llvm::Type*> argTypes;
			for (auto arg: node->args)
			{
				auto* argType = this->encode_type(arg.second);
				if(argType == nullptr)
					return false;
				argTypes.push_back(argType->handle);
			}
			
			llvm::FunctionType* procType = llvm::FunctionType::get(retType->handle, argTypes, false);
			auto handler = procType->getPointerTo();
			auto defval = llvm::ConstantPointerNull::get(handler);
			this->scope->addType(node->name, module->new_type(node->name, handler, defval));
			
			return true;
		}
		
		bool encode_stmt_symbol_def(struct ast_stmt_symbol_def_t* node)
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
			
			auto value = llvm_model_t::get_value(builder, expr->value);
			
			llvm::Type* stype = nullptr;
			llvm::Type* vtype = value->getType();
			if(type != nullptr)
			{
				stype = type->handle;
				do
				{
					if(stype == vtype)
						break;
					if(vtype->canLosslesslyBitCastTo(stype))
						break;
					if(vtype->isPointerTy() && vtype->getPointerElementType() == stype)
					{
						stype = type->handle = vtype;
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
			
			llvm::Value* symbol = builder.CreateAlloca(stype);
			builder.CreateStore(value, symbol);
			symbol = llvm_model_t::ref_value(builder, symbol);
			symbol->setName(node->name.cstr());
			
			auto symbolE = module->new_expr(symbol, expr->type);
			if(!scope->addSymbol(node->name, symbolE))
			{
				printf("There is a symbol named %s in this scope.\n", node->name.cstr());
				return false;
			}
			
			return true;
		}
		
		bool encode_stmt_break(struct ast_stmt_break_t* node)
		{
			if(node == nullptr)
				return false;
			
			if(this->breakPoint == nullptr)
				return false;
			
			builder.CreateBr(this->breakPoint);
			
			return true;
		}
		
		bool encode_stmt_continue(struct ast_stmt_continue_t* node)
		{
			if(node == nullptr)
				return false;
			
			if(this->continuePoint == nullptr)
				return false;
			
			builder.CreateBr(this->continuePoint);
			
			return true;
		}
		
		bool encode_stmt_return(struct ast_stmt_return_t* node)
		{
			if(node == nullptr)
				return false;
			
			auto expectedRetType = this->scope->func->getFunctionType()->getReturnType();
			
			if(node->value == nullptr)
			{
				if(!expectedRetType->isVoidTy())
				{
					printf("The function must return a value.\n");
					return false;
				}
				
				builder.CreateRetVoid();
				return true;
			}
			
			auto expr = this->encode_expr(node->value);
			if(expr == nullptr)
			{
				printf("Invalid ret value.\n");
				return false;
			}
			auto value = llvm_model_t::get_value(builder, expr->value);
			auto actureRetType = value->getType();
			if(actureRetType != expectedRetType && !actureRetType->canLosslesslyBitCastTo(expectedRetType))
			{
				printf("The type of return value can't cast to return type of function.\n");
				return false;
			}
			
			builder.CreateRet(value);
			
			return true;
		}
		
		bool encode_stmt_if(struct ast_stmt_if_t* node)
		{
			if(node == nullptr)
				return false;
			
			llvm::BasicBlock* if_true = llvm::BasicBlock::Create(context, "if.true", this->scope->func);
			llvm::BasicBlock* if_false = llvm::BasicBlock::Create(context, "if.false", this->scope->func);
			llvm::BasicBlock* if_end = llvm::BasicBlock::Create(context, "if.end", this->scope->func);
			
			auto condE = this->encode_expr(node->cond);
			if(condE == nullptr)
				return false;
			auto condV = llvm_model_t::get_value(builder, condE->value);
			if(!condV->getType()->isIntegerTy(1))
			{
				printf("The label 'if.cond' need a bool value.\n");
				return false;
			}
			builder.CreateCondBr(condV, if_true, if_false);
			
			// if-true
			builder.SetInsertPoint(if_true);
			if(node->branch_true != nullptr)
			{
				if(!this->encode_stmt(node->branch_true))
					return false;
				auto lastBlock = builder.GetInsertBlock();
				if(lastBlock != if_true && !lastBlock->back().isTerminator())
				{
					builder.CreateBr(if_end);
				}
			}
			if(!if_true->back().isTerminator())
			{
				builder.CreateBr(if_end);
			}
			
			// if-false
			builder.SetInsertPoint(if_false);
			if(node->branch_false != nullptr)
			{
				if(!this->encode_stmt(node->branch_false))
					return false;
				auto lastBlock = builder.GetInsertBlock();
				if(lastBlock != if_false && !lastBlock->back().isTerminator())
				{
					builder.CreateBr(if_end);
				}
			}
			if(!if_false->back().isTerminator())
			{
				builder.CreateBr(if_end);
			}
			
			builder.SetInsertPoint(if_end);
			
			return true;
		}
		
		bool encode_stmt_loop(struct ast_stmt_loop_t* node)
		{
			if(node == nullptr)
				return false;
			
			this->pushScope();
			
			llvm::BasicBlock* loop_cond = llvm::BasicBlock::Create(context, "loop.cond", this->scope->func);
			llvm::BasicBlock* loop_step = llvm::BasicBlock::Create(context, "loop.step", this->scope->func);
			llvm::BasicBlock* loop_body = llvm::BasicBlock::Create(context, "loop.body", this->scope->func);
			llvm::BasicBlock* loop_end = llvm::BasicBlock::Create(context, "loop.end", this->scope->func);
			
			auto oldContinuePoint = this->continuePoint;
			auto oldBreakPoint = this->breakPoint;
			this->continuePoint = loop_step;
			this->breakPoint = loop_end;
			
			if(!this->encode_stmt(node->init))
				return false;
			builder.CreateBr(loop_cond);
			
			builder.SetInsertPoint(loop_cond);
			auto condE = this->encode_expr(node->cond);
			if(condE == nullptr)
				return false;
			auto condV = llvm_model_t::get_value(builder, condE->value);
			if(!condV->getType()->isIntegerTy(1))
			{
				printf("The label 'for.cond' need a bool value.\n");
				return false;
			}
			builder.CreateCondBr(condV, loop_body, loop_end);
			
			builder.SetInsertPoint(loop_body);
			if(node->body != nullptr)
			{
				if(!this->encode_stmt(node->body))
					return false;
				auto& lastOp = loop_body->back();
				if(!lastOp.isTerminator())
				{
					builder.CreateBr(loop_step);
				}
			}
			if(!loop_body->back().isTerminator())
			{
				builder.CreateBr(loop_step);
			}
			
			builder.SetInsertPoint(loop_step);
			if(!this->encode_stmt(node->step))
				return false;
			builder.CreateBr(loop_cond);
			
			builder.SetInsertPoint(loop_end);
			
			this->continuePoint = oldContinuePoint;
			this->breakPoint = oldBreakPoint;
			
			this->popScope();
			
			return true;
		}
		
		bool encode_stmt_block(struct ast_stmt_block_t* node)
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
		
		bool encode_stmt_assign(struct ast_stmt_assign_t* node)
		{
			if(node == nullptr)
				return false;
			
			auto leftE = this->encode_expr(node->left);
			auto rightE = this->encode_expr(node->right);
			if(leftE == nullptr || rightE == nullptr)
				return false;
			
			auto ptr = llvm_model_t::ref_value(builder, leftE->value);
			auto val = llvm_model_t::get_value(builder, rightE->value);
			builder.CreateStore(val, ptr);
			
			return true;
		}
		
		bool encode_stmt_call(struct ast_stmt_call_t* node)
		{
			if(node == nullptr)
				return false;
			
			if(!this->encode_expr_func_ref(node->expr))
				return false;
			
			return true;
		}
	};
	
	llvm_module_t* llvm_encode(llvm::LLVMContext& context, ast_module_t* module)
	{
		llvm_coder_t coder(context);
		return coder.encode(module);
	}
_EndNamespace(eokas)
