#include "coder.h"
#include "models.h"
#include "scope.h"
#include "expr.h"

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
		llvm::LLVMContext& llvm_context;
		llvm::IRBuilder<> llvm_builder;
		llvm::Module* llvm_module;
		llvm_model_t model;
		std::vector<llvm_expr_t*> exprs;
		llvm_scope_t* root;
		llvm_scope_t* scope;
		llvm::Function* func;
		llvm::BasicBlock* continuePoint;
		llvm::BasicBlock* breakPoint;
		
		std::map<llvm::Type*, ast_stmt_struct_def_t*> structs;
		
		llvm::Value* const_zero;
		
		explicit llvm_coder_t(llvm::LLVMContext& llvm_context) : llvm_context(llvm_context), llvm_builder(llvm_context), llvm_module(nullptr), model(llvm_context)
		{
			this->root = new llvm_scope_t(nullptr);
			this->scope = this->root;
			this->func = nullptr;
			this->continuePoint = nullptr;
			this->breakPoint = nullptr;
			
			const_zero = llvm::ConstantInt::get(llvm_context, llvm::APInt(32, 0));
		}
		
		virtual ~llvm_coder_t()
		{
			_DeletePointer(this->scope);
			_DeleteList(this->exprs);
		}
		
		llvm_expr_t* new_expr(llvm::Value* value, llvm::Type* type = nullptr)
		{
			auto* expr = new llvm_expr_t(value, type);
			this->exprs.push_back(expr);
			return expr;
		}
		
		void pushScope()
		{
			this->scope = this->scope->addChild();
		}
		
		void popScope()
		{
			this->scope = this->scope->parent;
		}
		
		llvm::Module* encode(struct ast_module_t* m)
		{
			this->llvm_module = new llvm::Module("eokas", llvm_context);
			
			model.declare_cfunc_puts(llvm_module);
			model.declare_cfunc_printf(llvm_module);
			model.declare_cfunc_sprintf(llvm_module);
			model.declare_cfunc_malloc(llvm_module);
			model.declare_cfunc_free(llvm_module);
			
			if(!this->encode_module(m))
			{
				_DeletePointer(this->llvm_module);
				return nullptr;
			}
			return this->llvm_module;
		}
		
		bool encode_module(struct ast_module_t* node)
		{
			if(node == nullptr)
				return false;
			
			this->pushScope();
			
			this->scope->types["void"] = model.type_void;
			this->scope->types["i8"] = model.type_i8;
			this->scope->types["i32"] = model.type_i32;
			this->scope->types["i64"] = model.type_i64;
			this->scope->types["u8"] = model.type_u8;
			this->scope->types["u32"] = model.type_u32;
			this->scope->types["u64"] = model.type_u64;
			this->scope->types["f32"] = model.type_f32;
			this->scope->types["f64"] = model.type_f64;
			this->scope->types["bool"] = model.type_bool;
			this->scope->types["string"] = model.type_string_ptr;
			
			this->scope->symbols["print"] = new llvm_expr_t(model.define_func_print(llvm_module));
			
			llvm::FunctionType* funcType = llvm::FunctionType::get(model.type_i32, false);
			this->func = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, "main", llvm_module);
			
			llvm::BasicBlock* entry = llvm::BasicBlock::Create(llvm_context, "entry", this->func);
			llvm_builder.SetInsertPoint(entry);
			
			for (auto& stmt: node->get_func()->body)
			{
				if(!this->encode_stmt(stmt))
					return false;
			}
			
			llvm_builder.CreateRet(const_zero);
			
			this->popScope();
			
			return true;
		}
		
		llvm::Type* encode_type(struct ast_type_t* node)
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
		
		llvm::Type* encode_type_ref(struct ast_type_ref_t* node)
		{
			if(node == nullptr)
			{
				printf("type node is null. \n");
				return nullptr;
			}
			
			const String& name = node->name;
			llvm::Type* type = this->scope->getType(name, true);
			return type;
		}
		
		llvm::Type* encode_type_array(struct ast_type_array_t* node)
		{
			if(node == nullptr)
			{
				printf("type node is null. \n");
				return nullptr;
			}
			
			llvm::Type* elementType = this->encode_type(node->elementType);
			if(elementType == nullptr)
				return nullptr;
			
			llvm::Type* type = llvm::ArrayType::get(elementType, node->length);
			
			return type;
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
			
			llvm::BasicBlock* trinary_begin = llvm::BasicBlock::Create(llvm_context, "trinary.begin", this->func);
			llvm::BasicBlock* trinary_true = llvm::BasicBlock::Create(llvm_context, "trinary.true", this->func);
			llvm::BasicBlock* trinary_false = llvm::BasicBlock::Create(llvm_context, "trinary.false", this->func);
			llvm::BasicBlock* trinary_end = llvm::BasicBlock::Create(llvm_context, "trinary.end", this->func);
			
			llvm_builder.CreateBr(trinary_begin);
			llvm_builder.SetInsertPoint(trinary_begin);
			llvm_expr_t* condE = this->encode_expr(node->cond);
			if(condE == nullptr)
				return nullptr;
			if(!condE->type->isIntegerTy(1))
			{
				printf("condition must be a bool value.\n");
				return nullptr;
			}
			auto condV = model.get_value(llvm_builder, condE->value);
			llvm_builder.CreateCondBr(condV, trinary_true, trinary_false);
			
			llvm_builder.SetInsertPoint(trinary_true);
			llvm_expr_t* trueE = this->encode_expr(node->branch_true);
			if(trueE == nullptr)
				return nullptr;
			llvm_builder.CreateBr(trinary_end);
			
			llvm_builder.SetInsertPoint(trinary_false);
			llvm_expr_t* falseE = this->encode_expr(node->branch_false);
			if(falseE == nullptr)
				return nullptr;
			llvm_builder.CreateBr(trinary_end);
			
			llvm_builder.SetInsertPoint(trinary_end);
			if(trueE->type != falseE->type)
			{
				printf("type of true-branch must be the same as false-branch.\n");
				return nullptr;
			}
			
			llvm::PHINode* phi = llvm_builder.CreatePHI(trueE->type, 2);
			phi->addIncoming(trueE->value, trinary_true);
			phi->addIncoming(falseE->value, trinary_false);
			
			return this->new_expr(phi);
		}
		
		llvm_expr_t* encode_expr_binary_type(struct ast_expr_binary_type_t* node)
		{
			if(node == nullptr)
				return nullptr;
			
			auto left = this->encode_expr(node->left);
			auto right = this->encode_type(node->right);
			if(left == nullptr || right == nullptr)
				return nullptr;
			
			auto lhs = model.get_value(llvm_builder, left->value);
			
			switch (node->op)
			{
				case ast_binary_oper_t::Is:
				{
					if(right == lhs->getType())
						return this->new_expr(llvm_builder.getTrue());
					else
						return this->new_expr(llvm_builder.getFalse());
				}
					break;
				case ast_binary_oper_t::As:
				{
					if(lhs->getType()->isSingleValueType())
					{
						if(right == model.type_string_ptr)
						{
							llvm::Value* value = model.value_to_string(llvm_module, this->func, llvm_builder, lhs);
							return this->new_expr(value);
						}
						return nullptr;
					}
					else
					{
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
		
		llvm_expr_t* encode_expr_binary_value(struct ast_expr_binary_value_t* node)
		{
			if(node == nullptr)
				return nullptr;
			
			auto left = this->encode_expr(node->left);
			auto right = this->encode_expr(node->right);
			if(left == nullptr || right == nullptr)
				return nullptr;
			
			auto lhs = model.get_value(llvm_builder, left->value);
			auto rhs = model.get_value(llvm_builder, right->value);
			
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
			
			return this->new_expr(llvm_builder.CreateOr(lhs, rhs));
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
			
			return this->new_expr(llvm_builder.CreateAnd(lhs, rhs));
		}
		
		llvm_expr_t* encode_expr_binary_eq(llvm::Value* lhs, llvm::Value* rhs)
		{
			auto ltype = lhs->getType();
			auto rtype = rhs->getType();
			
			if(ltype->isIntegerTy() && rtype->isIntegerTy())
				return this->new_expr(llvm_builder.CreateICmpEQ(lhs, rhs));
			
			if(ltype->isFloatingPointTy() && rtype->isFloatingPointTy())
				return this->new_expr(llvm_builder.CreateFCmpOEQ(lhs, rhs));
			
			if(ltype->isPointerTy() && rtype->isPointerTy())
			{
				return this->new_expr(llvm_builder.CreateICmpEQ(llvm_builder.CreatePtrToInt(lhs, llvm::Type::getInt64Ty(llvm_context)), llvm_builder.CreatePtrToInt(rhs, llvm::Type::getInt64Ty(llvm_context))));
			}
			
			if(ltype->isIntegerTy() && rtype->isFloatingPointTy())
			{
				return this->new_expr(llvm_builder.CreateFCmpOEQ(llvm_builder.CreateSIToFP(lhs, llvm::Type::getDoubleTy(llvm_context)), rhs));
			}
			
			if(ltype->isFloatingPointTy() && rtype->isIntegerTy())
			{
				return this->new_expr(llvm_builder.CreateFCmpOEQ(lhs, llvm_builder.CreateSIToFP(rhs, llvm::Type::getDoubleTy(llvm_context))));
			}
			
			printf("Type of LHS or RHS is invalid.");
			return nullptr;
		}
		
		llvm_expr_t* encode_expr_binary_ne(llvm::Value* lhs, llvm::Value* rhs)
		{
			auto ltype = lhs->getType();
			auto rtype = rhs->getType();
			
			if(ltype->isIntegerTy() && rtype->isIntegerTy())
				return this->new_expr(llvm_builder.CreateICmpNE(lhs, rhs));
			
			if(ltype->isFloatingPointTy() && rtype->isFloatingPointTy())
				return this->new_expr(llvm_builder.CreateFCmpONE(lhs, rhs));
			
			if(ltype->isPointerTy() && rtype->isPointerTy())
			{
				return this->new_expr(llvm_builder.CreateICmpNE(llvm_builder.CreatePtrToInt(lhs, llvm::Type::getInt64Ty(llvm_context)), llvm_builder.CreatePtrToInt(rhs, llvm::Type::getInt64Ty(llvm_context))));
			}
			
			if(ltype->isIntegerTy() && rtype->isFloatingPointTy())
			{
				return this->new_expr(llvm_builder.CreateFCmpONE(llvm_builder.CreateSIToFP(lhs, llvm::Type::getDoubleTy(llvm_context)), rhs));
			}
			
			if(ltype->isFloatingPointTy() && rtype->isIntegerTy())
			{
				return this->new_expr(llvm_builder.CreateFCmpONE(lhs, llvm_builder.CreateSIToFP(rhs, llvm::Type::getDoubleTy(llvm_context))));
			}
			
			printf("Type of LHS or RHS is invalid.");
			return nullptr;
		}
		
		llvm_expr_t* encode_expr_binary_le(llvm::Value* lhs, llvm::Value* rhs)
		{
			auto ltype = lhs->getType();
			auto rtype = rhs->getType();
			
			if(ltype->isIntegerTy() && rtype->isIntegerTy())
				return this->new_expr(llvm_builder.CreateICmpSLE(lhs, rhs));
			
			if(ltype->isFloatingPointTy() && rtype->isFloatingPointTy())
				return this->new_expr(llvm_builder.CreateFCmpOLE(lhs, rhs));
			
			if(ltype->isPointerTy() && rtype->isPointerTy())
			{
				return this->new_expr(llvm_builder.CreateICmpULE(llvm_builder.CreatePtrToInt(lhs, llvm::Type::getInt64Ty(llvm_context)), llvm_builder.CreatePtrToInt(rhs, llvm::Type::getInt64Ty(llvm_context))));
			}
			
			if(ltype->isIntegerTy() && rtype->isFloatingPointTy())
			{
				return this->new_expr(llvm_builder.CreateFCmpOLE(llvm_builder.CreateSIToFP(lhs, llvm::Type::getDoubleTy(llvm_context)), rhs));
			}
			
			if(ltype->isFloatingPointTy() && rtype->isIntegerTy())
			{
				return this->new_expr(llvm_builder.CreateFCmpOLE(lhs, llvm_builder.CreateSIToFP(rhs, llvm::Type::getDoubleTy(llvm_context))));
			}
			
			printf("Type of LHS or RHS is invalid.");
			return nullptr;
		}
		
		llvm_expr_t* encode_expr_binary_ge(llvm::Value* lhs, llvm::Value* rhs)
		{
			auto ltype = lhs->getType();
			auto rtype = rhs->getType();
			
			if(ltype->isIntegerTy() && rtype->isIntegerTy())
				return this->new_expr(llvm_builder.CreateICmpSGE(lhs, rhs));
			
			if(ltype->isFloatingPointTy() && rtype->isFloatingPointTy())
				return this->new_expr(llvm_builder.CreateFCmpOGE(lhs, rhs));
			
			if(ltype->isPointerTy() && rtype->isPointerTy())
			{
				return this->new_expr(llvm_builder.CreateICmpUGE(llvm_builder.CreatePtrToInt(lhs, llvm::Type::getInt64Ty(llvm_context)), llvm_builder.CreatePtrToInt(rhs, llvm::Type::getInt64Ty(llvm_context))));
			}
			
			if(ltype->isIntegerTy() && rtype->isFloatingPointTy())
			{
				return this->new_expr(llvm_builder.CreateFCmpOGE(llvm_builder.CreateSIToFP(lhs, llvm::Type::getDoubleTy(llvm_context)), rhs));
			}
			
			if(ltype->isFloatingPointTy() && rtype->isIntegerTy())
			{
				return this->new_expr(llvm_builder.CreateFCmpOGE(lhs, llvm_builder.CreateSIToFP(rhs, llvm::Type::getDoubleTy(llvm_context))));
			}
			
			printf("Type of LHS or RHS is invalid.");
			return nullptr;
		}
		
		llvm_expr_t* encode_expr_binary_lt(llvm::Value* lhs, llvm::Value* rhs)
		{
			auto ltype = lhs->getType();
			auto rtype = rhs->getType();
			
			if(ltype->isIntegerTy() && rtype->isIntegerTy())
				return this->new_expr(llvm_builder.CreateICmpSLT(lhs, rhs));
			
			if(ltype->isFloatingPointTy() && rtype->isFloatingPointTy())
				return this->new_expr(llvm_builder.CreateFCmpOLT(lhs, rhs));
			
			if(ltype->isPointerTy() && rtype->isPointerTy())
			{
				return this->new_expr(llvm_builder.CreateICmpULT(llvm_builder.CreatePtrToInt(lhs, llvm::Type::getInt64Ty(llvm_context)), llvm_builder.CreatePtrToInt(rhs, llvm::Type::getInt64Ty(llvm_context))));
			}
			
			if(ltype->isIntegerTy() && rtype->isFloatingPointTy())
			{
				return this->new_expr(llvm_builder.CreateFCmpOLT(llvm_builder.CreateSIToFP(lhs, llvm::Type::getDoubleTy(llvm_context)), rhs));
			}
			
			if(ltype->isFloatingPointTy() && rtype->isIntegerTy())
			{
				return this->new_expr(llvm_builder.CreateFCmpOLT(lhs, llvm_builder.CreateSIToFP(rhs, llvm::Type::getDoubleTy(llvm_context))));
			}
			
			printf("Type of LHS or RHS is invalid.");
			return nullptr;
		}
		
		llvm_expr_t* encode_expr_binary_gt(llvm::Value* lhs, llvm::Value* rhs)
		{
			auto ltype = lhs->getType();
			auto rtype = rhs->getType();
			
			if(ltype->isIntegerTy() && rtype->isIntegerTy())
				return this->new_expr(llvm_builder.CreateICmpSGT(lhs, rhs));
			
			if(ltype->isFloatingPointTy() && rtype->isFloatingPointTy())
				return this->new_expr(llvm_builder.CreateFCmpOGT(lhs, rhs));
			
			if(ltype->isPointerTy() && rtype->isPointerTy())
			{
				return this->new_expr(llvm_builder.CreateICmpUGT(llvm_builder.CreatePtrToInt(lhs, llvm::Type::getInt64Ty(llvm_context)), llvm_builder.CreatePtrToInt(rhs, llvm::Type::getInt64Ty(llvm_context))));
			}
			
			if(ltype->isIntegerTy() && rtype->isFloatingPointTy())
			{
				return this->new_expr(llvm_builder.CreateFCmpOGT(llvm_builder.CreateSIToFP(lhs, llvm::Type::getDoubleTy(llvm_context)), rhs));
			}
			
			if(ltype->isFloatingPointTy() && rtype->isIntegerTy())
			{
				return this->new_expr(llvm_builder.CreateFCmpOGT(lhs, llvm_builder.CreateSIToFP(rhs, llvm::Type::getDoubleTy(llvm_context))));
			}
			
			printf("Type of LHS or RHS is invalid.");
			return nullptr;
		}
		
		llvm_expr_t* encode_expr_binary_add(llvm::Value* lhs, llvm::Value* rhs)
		{
			auto ltype = lhs->getType();
			auto rtype = rhs->getType();
			
			if(ltype->isIntegerTy() && rtype->isIntegerTy())
				return this->new_expr(llvm_builder.CreateAdd(lhs, rhs));
			
			if(ltype->isFloatingPointTy() && rtype->isFloatingPointTy())
				return this->new_expr(llvm_builder.CreateFAdd(lhs, rhs));
			
			if(ltype->isIntegerTy() && rtype->isFloatingPointTy())
			{
				return this->new_expr(llvm_builder.CreateFAdd(llvm_builder.CreateSIToFP(lhs, llvm::Type::getDoubleTy(llvm_context)), rhs));
			}
			
			if(ltype->isFloatingPointTy() && rtype->isIntegerTy())
			{
				return this->new_expr(llvm_builder.CreateFAdd(lhs, llvm_builder.CreateSIToFP(rhs, llvm::Type::getDoubleTy(llvm_context))));
			}
			
			printf("Type of LHS or RHS is invalid.");
			return nullptr;
		}
		
		llvm_expr_t* encode_expr_binary_sub(llvm::Value* lhs, llvm::Value* rhs)
		{
			auto ltype = lhs->getType();
			auto rtype = rhs->getType();
			
			if(ltype->isIntegerTy() && rtype->isIntegerTy())
				return this->new_expr(llvm_builder.CreateSub(lhs, rhs));
			
			if(ltype->isFloatingPointTy() && rtype->isFloatingPointTy())
				return this->new_expr(llvm_builder.CreateFSub(lhs, rhs));
			
			if(ltype->isIntegerTy() && rtype->isFloatingPointTy())
			{
				return this->new_expr(llvm_builder.CreateFSub(llvm_builder.CreateSIToFP(lhs, llvm::Type::getDoubleTy(llvm_context)), rhs));
			}
			
			if(ltype->isFloatingPointTy() && rtype->isIntegerTy())
			{
				return this->new_expr(llvm_builder.CreateFSub(lhs, llvm_builder.CreateSIToFP(rhs, llvm::Type::getDoubleTy(llvm_context))));
			}
			
			printf("Type of LHS or RHS is invalid.");
			return nullptr;
		}
		
		llvm_expr_t* encode_expr_binary_mul(llvm::Value* lhs, llvm::Value* rhs)
		{
			auto ltype = lhs->getType();
			auto rtype = rhs->getType();
			
			if(ltype->isIntegerTy() && rtype->isIntegerTy())
				return this->new_expr(llvm_builder.CreateMul(lhs, rhs));
			
			if(ltype->isFloatingPointTy() && rtype->isFloatingPointTy())
				return this->new_expr(llvm_builder.CreateFMul(lhs, rhs));
			
			if(ltype->isIntegerTy() && rtype->isFloatingPointTy())
			{
				return this->new_expr(llvm_builder.CreateFMul(llvm_builder.CreateSIToFP(lhs, llvm::Type::getDoubleTy(llvm_context)), rhs));
			}
			
			if(ltype->isFloatingPointTy() && rtype->isIntegerTy())
			{
				return this->new_expr(llvm_builder.CreateFMul(lhs, llvm_builder.CreateSIToFP(rhs, llvm::Type::getDoubleTy(llvm_context))));
			}
			
			printf("Type of LHS or RHS is invalid.");
			return nullptr;
		}
		
		llvm_expr_t* encode_expr_binary_div(llvm::Value* lhs, llvm::Value* rhs)
		{
			auto ltype = lhs->getType();
			auto rtype = rhs->getType();
			
			if(ltype->isIntegerTy() && rtype->isIntegerTy())
				return this->new_expr(llvm_builder.CreateSDiv(lhs, rhs));
			
			if(ltype->isFloatingPointTy() && rtype->isFloatingPointTy())
				return this->new_expr(llvm_builder.CreateFMul(lhs, rhs));
			
			if(ltype->isIntegerTy() && rtype->isFloatingPointTy())
			{
				return this->new_expr(llvm_builder.CreateFDiv(llvm_builder.CreateSIToFP(lhs, llvm::Type::getDoubleTy(llvm_context)), rhs));
			}
			
			if(ltype->isFloatingPointTy() && rtype->isIntegerTy())
			{
				return this->new_expr(llvm_builder.CreateFDiv(lhs, llvm_builder.CreateSIToFP(rhs, llvm::Type::getDoubleTy(llvm_context))));
			}
			
			printf("Type of LHS or RHS is invalid.");
			return nullptr;
		}
		
		llvm_expr_t* encode_expr_binary_mod(llvm::Value* lhs, llvm::Value* rhs)
		{
			auto ltype = lhs->getType();
			auto rtype = rhs->getType();
			
			if(ltype->isIntegerTy() && rtype->isIntegerTy())
				return this->new_expr(llvm_builder.CreateSRem(lhs, rhs));
			
			if(ltype->isFloatingPointTy() && rtype->isFloatingPointTy())
				return this->new_expr(llvm_builder.CreateFRem(lhs, rhs));
			
			if(ltype->isIntegerTy() && rtype->isFloatingPointTy())
			{
				return this->new_expr(llvm_builder.CreateFRem(llvm_builder.CreateSIToFP(lhs, llvm::Type::getDoubleTy(llvm_context)), rhs));
			}
			
			if(ltype->isFloatingPointTy() && rtype->isIntegerTy())
			{
				return this->new_expr(llvm_builder.CreateFRem(lhs, llvm_builder.CreateSIToFP(rhs, llvm::Type::getDoubleTy(llvm_context))));
			}
			
			printf("Type of LHS or RHS is invalid.");
			return nullptr;
		}
		
		llvm_expr_t* encode_expr_binary_bitand(llvm::Value* lhs, llvm::Value* rhs)
		{
			auto ltype = lhs->getType();
			auto rtype = rhs->getType();
			
			if(ltype->isIntegerTy() && rtype->isIntegerTy())
				return this->new_expr(llvm_builder.CreateAnd(lhs, rhs));
			
			printf("Type of LHS or RHS is invalid.");
			return nullptr;
		}
		
		llvm_expr_t* encode_expr_binary_bitor(llvm::Value* lhs, llvm::Value* rhs)
		{
			auto ltype = lhs->getType();
			auto rtype = rhs->getType();
			
			if(ltype->isIntegerTy() && rtype->isIntegerTy())
				return this->new_expr(llvm_builder.CreateOr(lhs, rhs));
			
			printf("Type of LHS or RHS is invalid.");
			return nullptr;
		}
		
		llvm_expr_t* encode_expr_binary_bitxor(llvm::Value* lhs, llvm::Value* rhs)
		{
			auto ltype = lhs->getType();
			auto rtype = rhs->getType();
			
			if(ltype->isIntegerTy() && rtype->isIntegerTy())
				return this->new_expr(llvm_builder.CreateXor(lhs, rhs));
			
			printf("Type of LHS or RHS is invalid.");
			return nullptr;
		}
		
		llvm_expr_t* encode_expr_binary_bitshl(llvm::Value* lhs, llvm::Value* rhs)
		{
			auto ltype = lhs->getType();
			auto rtype = rhs->getType();
			
			if(ltype->isIntegerTy() && rtype->isIntegerTy())
				return this->new_expr(llvm_builder.CreateShl(lhs, rhs));
			
			printf("Type of LHS or RHS is invalid.");
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
				return this->new_expr(llvm_builder.CreateLShr(lhs, rhs));
			}
			
			printf("Type of LHS or RHS is invalid.");
			return nullptr;
		}
		
		llvm_expr_t* encode_expr_unary(struct ast_expr_unary_t* node)
		{
			if(node == nullptr)
				return nullptr;
			
			auto right = this->encode_expr(node->right);
			if(right == nullptr)
				return nullptr;
			
			auto rhs = model.get_value(llvm_builder, right->value);
			
			switch (node->op)
			{
				case ast_unary_oper_t::Pos:
					return this->new_expr(rhs);
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
				return this->new_expr(llvm_builder.CreateNeg(rhs));
			
			if(rtype->isFloatingPointTy())
				return this->new_expr(llvm_builder.CreateFNeg(rhs));
			
			printf("Type of RHS is invalid.");
			return nullptr;
		}
		
		llvm_expr_t* encode_expr_unary_not(llvm::Value* rhs)
		{
			auto rtype = rhs->getType();
			
			if(rtype->isIntegerTy() && rtype->getIntegerBitWidth() == 1)
				return this->new_expr(llvm_builder.CreateNot(rhs));
			
			printf("Type of RHS is invalid.");
			return nullptr;
		}
		
		llvm_expr_t* encode_expr_unary_flip(llvm::Value* rhs)
		{
			auto rtype = rhs->getType();
			
			if(rtype->isIntegerTy())
			{
				return this->new_expr(llvm_builder.CreateXor(rhs, llvm::ConstantInt::get(rtype, llvm::APInt(rtype->getIntegerBitWidth(), 0xFFFFFFFF))));
			}
			printf("Type of RHS is invalid.");
			return nullptr;
		}
		
		llvm_expr_t* encode_expr_int(struct ast_expr_int_t* node)
		{
			if(node == nullptr)
				return nullptr;
			
			u64_t vals = *((u64_t*) &(node->value));
			u32_t bits = vals>0xFFFFFFFF ? 64 : 32;
			
			return this->new_expr(llvm::ConstantInt::get(llvm_context, llvm::APInt(bits, node->value)));
		}
		
		llvm_expr_t* encode_expr_float(struct ast_expr_float_t* node)
		{
			if(node == nullptr)
				return nullptr;
			
			return this->new_expr(llvm::ConstantFP::get(llvm_context, llvm::APFloat(node->value)));
		}
		
		llvm_expr_t* encode_expr_bool(struct ast_expr_bool_t* node)
		{
			if(node == nullptr)
				return nullptr;
			return this->new_expr(llvm::ConstantInt::getBool(llvm_context, node->value));
		}
		
		llvm_expr_t* encode_expr_string(struct ast_expr_string_t* node)
		{
			if(node == nullptr)
				return nullptr;
			
			auto str = model.make(llvm_module, this->func, llvm_builder, model.type_string);
			auto ptr = llvm_builder.CreateStructGEP(str, 0);
			auto val = llvm_builder.CreateGlobalString(node->value.cstr());
			llvm_builder.CreateStore(val, ptr);
			return this->new_expr(str);
		}
		
		llvm_expr_t* encode_expr_symbol_ref(struct ast_expr_symbol_ref_t* node)
		{
			if(node == nullptr)
				return nullptr;
			
			auto symbol = this->scope->getSymbol(node->name, true);
			if(symbol == nullptr)
			{
				printf("symbol '%s' is undefined.\n", node->name.cstr());
				return nullptr;
			}
			
			return symbol;
		}
		
		llvm_expr_t* encode_expr_func_def(struct ast_expr_func_def_t* node)
		{
			if(node == nullptr)
				return nullptr;
			
			llvm::Type* retType = this->encode_type(node->type);
			if(retType == nullptr)
				return nullptr;
			
			std::vector<llvm::Type*> argTypes;
			for (auto arg: node->args)
			{
				llvm::Type* argType = this->encode_type(arg->type);
				if(argType == nullptr)
					return nullptr;
				argTypes.push_back(argType);
			}
			
			llvm::FunctionType* funcType = llvm::FunctionType::get(retType, argTypes, false);
			llvm::Function* func = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, "", llvm_module);
			u32_t index = 0;
			for (auto& arg: func->args())
			{
				const char* name = node->args[index++]->name.cstr();
				arg.setName(name);
			}
			
			auto oldFunc = this->func;
			auto oldIB = llvm_builder.GetInsertBlock();
			this->func = func;
			this->pushScope();
			{
				llvm::BasicBlock* entry = llvm::BasicBlock::Create(llvm_context, "entry", func);
				llvm_builder.SetInsertPoint(entry);
				
				for (auto& arg: func->args())
				{
					const char* name = arg.getName().data();
					auto expr = this->new_expr(&arg);
					auto pair = std::make_pair(String(name), expr);
					this->scope->symbols.insert(pair);
				}
				
				for (auto& stmt: node->body)
				{
					if(!this->encode_stmt(stmt))
						return nullptr;
				}
				
				llvm_builder.CreateRetVoid();
			}
			
			this->popScope();
			this->func = oldFunc;
			llvm_builder.SetInsertPoint(oldIB);
			
			return this->new_expr(func, func->getFunctionType());
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
			
			auto funcPtr = model.get_value(llvm_builder, funcExpr->value);
			auto funcType = llvm::cast<llvm::FunctionType>(funcExpr->type);
			
			std::vector<llvm::Value*> params;
			for (auto i = 0; i<node->args.size(); i++)
			{
				auto* paramT = funcType->getParamType(i);
				auto* paramE = this->encode_expr(node->args.at(i));
				if(paramE == nullptr)
					return nullptr;
				if(paramE->type != paramT && !paramE->type->canLosslesslyBitCastTo(paramT))
				{
					printf("the type of param[%d] can't cast to the param type of function.\n", i);
					return nullptr;
				}
				auto paramV = model.ref_value(llvm_builder, paramE->value);
				params.push_back(paramV);
			}
			
			llvm::Value* retval = llvm_builder.CreateCall(funcType, funcPtr, params);
			return this->new_expr(retval);
		}
		
		llvm_expr_t* encode_expr_array_def(struct ast_expr_array_def_t* node)
		{
			if(node == nullptr)
				return nullptr;
			
			printf("1 \n");
			std::vector<llvm::Constant*> arrayItems;
			llvm::Type* itemType = nullptr;
			for (auto item: node->items)
			{
				printf("2 \n");
				auto itemE = this->encode_expr(item);
				if(itemE == nullptr)
					return nullptr;
				auto itemV = llvm::cast<llvm::Constant>(itemE->value);
				if(itemV == nullptr)
					return nullptr;
				
				auto thisItemType = itemV->getType();
				if(itemType == nullptr)
				{
					itemType = thisItemType;
				}
				
				arrayItems.push_back(itemV);
			}
			
			if(arrayItems.empty() || itemType == nullptr)
				itemType = llvm::Type::getInt32Ty(llvm_context);
			
			printf("3 \n");
			auto arrayType = llvm::ArrayType::get(itemType, 0);
			auto arrayValue = llvm::ConstantArray::get(arrayType, arrayItems);
			
			printf("4 \n");
			return this->new_expr(arrayValue);
		}
		
		llvm_expr_t* encode_expr_index_ref(struct ast_expr_index_ref_t* node)
		{
			if(node == nullptr)
				return nullptr;
			
			auto objE = this->encode_expr(node->obj);
			auto keyE = this->encode_expr(node->key);
			if(objE == nullptr || keyE == nullptr)
				return nullptr;
			
			if(objE->type->isArrayTy() && keyE->type->isIntegerTy())
			{
				llvm::Value* value = llvm_builder.CreateGEP(objE->value, keyE->value);
				return this->new_expr(value);
			}
			
			return nullptr;
		}
		
		llvm_expr_t* encode_expr_object_def(struct ast_expr_object_def_t* node)
		{
			if(node == nullptr)
				return nullptr;
			
			auto* structRefType = llvm::cast<llvm::PointerType>(this->encode_type(node->type));
			if(structRefType == nullptr)
				return nullptr;
			
			auto* structType = llvm::cast<llvm::StructType>(structRefType->getElementType());
			if(structType == nullptr)
				return nullptr;
			
			auto iter = this->structs.find(structType);
			if(iter == this->structs.end())
				return nullptr;
			ast_stmt_struct_def_t* structDef = iter->second;
			
			for (auto& mem: node->members)
			{
				if(structDef->members.find(mem.first) == structDef->members.end())
				{
					printf("object member '%s' is not defined in struct.", mem.first.cstr());
					return nullptr;
				}
			}
			
			// make object
			llvm::Value* objectValue = model.make(llvm_module, this->func, llvm_builder, structType);
			
			// call constructor.
			auto structInitName = String::format("Type.%s.Init", structDef->name.cstr());
			auto structInitExpr = this->scope->getSymbol(structInitName, true);
			if(structInitExpr == nullptr)
			{
				printf("Can not find type constructor: %s\n", structInitName.cstr());
				return nullptr;
			}
			auto funcPtr = model.get_value(llvm_builder, structInitExpr->value);
			auto funcType = llvm::cast<llvm::FunctionType>(structInitExpr->type);
			llvm_builder.CreateCall(funcType, funcPtr, {objectValue});
			
			u32_t index = -1;
			for (auto& mem: structDef->members)
			{
				index += 1;
				auto memNode = node->members.find(mem.first);
				llvm::Value* memV = nullptr;
				if(memNode != node->members.end())
				{
					auto memE = this->encode_expr(memNode->second);
					if(memE == nullptr)
						return nullptr;
					memV = model.get_value(llvm_builder, memE->value);
				}
				
				if(memV)
				{
					llvm::Value* memPtr = llvm_builder.CreateStructGEP(structType, objectValue, index, mem.first.cstr());
					llvm_builder.CreateStore(memV, memPtr);
				}
			}
			
			return this->new_expr(objectValue);
		}
		
		llvm_expr_t* encode_expr_object_ref(struct ast_expr_object_ref_t* node)
		{
			if(node == nullptr)
				return nullptr;
			
			auto instanceE = this->encode_expr(node->obj);
			auto key = llvm_builder.CreateGlobalString(node->key.cstr());
			if(instanceE == nullptr || key == nullptr)
				return nullptr;
			
			auto instanceV = model.ref_value(llvm_builder, instanceE->value);
			auto instanceType = instanceV->getType();
			if(!(instanceType->isPointerTy() && instanceType->getPointerElementType()->isStructTy()))
			{
				printf("instance is not a object reference.");
				return nullptr;
			}
			auto structType = instanceType->getPointerElementType();
			auto structIter = this->structs.find(structType);
			if(structIter == this->structs.end())
				return nullptr;
			
			auto structAST = structIter->second;
			
			u32_t index = -1;
			for (auto& mem: structAST->members)
			{
				index += 1;
				if(mem.second->name == node->key)
					break;
			}
			if(index<0)
				return nullptr;
			
			llvm::Value* value = llvm_builder.CreateStructGEP(structType, instanceV, index);
			return this->new_expr(value);
		}
		
		bool encode_stmt(struct ast_stmt_t* node)
		{
			if(node == nullptr)
				return false;
			
			switch (node->category)
			{
				case ast_node_category_t::stmt_schema_def:
					return this->encode_stmt_schema_def(dynamic_cast<ast_stmt_schema_def_t*>(node));
				case ast_node_category_t::stmt_struct_def:
					return this->encode_stmt_struct_def(dynamic_cast<ast_stmt_struct_def_t*>(node));
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
				case ast_node_category_t::stmt_while:
					return this->encode_stmt_while(dynamic_cast<ast_stmt_while_t*>(node));
				case ast_node_category_t::stmt_for:
					return this->encode_stmt_for(dynamic_cast<ast_stmt_for_t*>(node));
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
		
		bool encode_stmt_schema_def(struct ast_stmt_schema_def_t* node)
		{
			if(node == nullptr)
				return false;
			
			const String& name = node->name;
			
			std::vector<llvm::Constant*> memberNames;
			std::vector<llvm::Type*> memberTypes;
			auto tokenNameType = llvm::ArrayType::get(llvm::Type::getInt8Ty(llvm_context), 256);
			auto metaType = llvm::ArrayType::get(tokenNameType, node->members.size() + 1);
			memberNames.push_back(llvm::ConstantDataArray::getString(llvm_context, "$_meta"));
			memberTypes.push_back(metaType);
			for (auto node: node->members)
			{
				auto mem = node.second;
				const auto& memName = mem->name.cstr();
				const auto& memType = this->encode_type(mem->type);
				if(memType == nullptr)
					return false;
				memberNames.push_back(llvm::ConstantDataArray::getString(llvm_context, memName));
				memberTypes.push_back(memType);
			}
			
			auto structType = llvm::StructType::get(llvm_context);
			structType->setBody(memberTypes);
			
			this->scope->types.insert(std::make_pair(name, structType));
			
			return true;
		}
		
		bool encode_stmt_struct_def(struct ast_stmt_struct_def_t* node)
		{
			if(node == nullptr)
				return false;
			
			const String& name = node->name;
			
			std::vector<llvm::Constant*> memberNames;
			std::vector<llvm::Type*> memberTypes;
			for (const auto& memNode: node->members)
			{
				auto mem = memNode.second;
				const auto& memName = mem->name.cstr();
				const auto& memType = this->encode_type(mem->type);
				if(memType == nullptr)
					return false;
				
				memberNames.push_back(llvm_builder.CreateGlobalString(memName));
				memberTypes.push_back(memType);
			}
			
			String structName = String::format("Type.%s.Struct", name.cstr());
			auto structType = llvm::StructType::create(llvm_context);
			structType->setName(structName.cstr());
			structType->setBody(memberTypes);
			
			llvm::Type* structPtrType = structType->getPointerTo();
			
			this->scope->types.insert(std::make_pair(name, structPtrType));
			this->structs.insert(std::make_pair(structType, node));
			
			String structInitName = String::format("Type.%s.Init", name.cstr());
			auto structInitType = llvm::FunctionType::get(model.type_void, {structPtrType}, false);
			auto structInitFunc = llvm::Function::Create(structInitType, llvm::Function::ExternalLinkage, structInitName.cstr(), llvm_module);
			
			auto* old_block = llvm_builder.GetInsertBlock();
			
			llvm::BasicBlock* entry = llvm::BasicBlock::Create(llvm_context, "entry", structInitFunc);
			llvm_builder.SetInsertPoint(entry);
			
			llvm::Value* obj = structInitFunc->getArg(0);
			
			u32_t index = -1;
			for (const auto& memNode: node->members)
			{
				index += 1;
				auto mem = memNode.second;
				if(mem->value != nullptr)
				{
					auto memE = this->encode_expr(mem->value);
					if(memE == nullptr)
						return false;
					auto memV = model.get_value(llvm_builder, memE->value);
					auto memP = llvm_builder.CreateStructGEP(structType, obj, index);
					llvm_builder.CreateStore(memV, memP);
				}
			}
			
			llvm_builder.CreateRetVoid();
			
			llvm_builder.SetInsertPoint(old_block);
			
			auto structInitExpr = this->new_expr(structInitFunc, structInitType);
			this->scope->symbols.insert(std::make_pair(structInitName, structInitExpr));
			
			return true;
		}
		
		bool encode_stmt_proc_def(struct ast_stmt_proc_def_t* node)
		{
			if(node == nullptr)
				return false;
			
			if(this->scope->getType(node->name, false) != nullptr)
				return false;
			
			llvm::Type* retType = this->encode_type(node->type);
			
			std::vector<llvm::Type*> argTypes;
			for (auto arg: node->args)
			{
				llvm::Type* argType = this->encode_type(arg.second);
				if(argType == nullptr)
					return false;
				argTypes.push_back(argType);
			}
			
			llvm::FunctionType* procType = llvm::FunctionType::get(retType, argTypes, false);
			
			this->scope->types[node->name] = procType;
			
			return true;
		}
		
		bool encode_stmt_symbol_def(struct ast_stmt_symbol_def_t* node)
		{
			if(node == nullptr)
				return false;
			
			if(this->scope->getSymbol(node->name, false) != nullptr)
				return false;
			
			auto type = this->encode_type(node->type);
			auto expr = this->encode_expr(node->value);
			if(expr == nullptr)
				return false;
			
			auto value = model.get_value(llvm_builder, expr->value);
			llvm::Type* vtype = value->getType();
			
			if(type != nullptr)
			{
				do
				{
					if(type == vtype)
						break;
					if(vtype->canLosslesslyBitCastTo(type))
						break;
					if(vtype->isPointerTy() && type == vtype->getPointerElementType())
					{
						type = vtype;
						break;
					}
					
					printf("type of value can not cast to the type of symbol.\n");
					return false;
				}
				while(false);
			}
			else
			{
				type = vtype;
			}
			
			if(type->isVoidTy())
			{
				printf("void type can't assign to a symbol.");
				return false;
			}
			// TODO: 校验类型合法性, 值类型是否遵循标记类型
			
			auto symbol = llvm_builder.CreateAlloca(type, nullptr, node->name.cstr());
			llvm_builder.CreateStore(value, symbol);
			auto symbolE = this->new_expr(symbol, expr->type);
			scope->symbols.insert(std::make_pair(node->name, symbolE));
			
			return true;
		}
		
		bool encode_stmt_break(struct ast_stmt_break_t* node)
		{
			if(node == nullptr)
				return false;
			
			if(this->breakPoint == nullptr)
				return false;
			
			llvm_builder.CreateBr(this->breakPoint);
			
			return true;
		}
		
		bool encode_stmt_continue(struct ast_stmt_continue_t* node)
		{
			if(node == nullptr)
				return false;
			
			if(this->continuePoint == nullptr)
				return false;
			
			llvm_builder.CreateBr(this->continuePoint);
			
			return true;
		}
		
		bool encode_stmt_return(struct ast_stmt_return_t* node)
		{
			if(node == nullptr)
				return false;
			
			auto expectedRetType = this->func->getFunctionType()->getReturnType();
			
			if(node->value == nullptr)
			{
				if(!expectedRetType->isVoidTy())
				{
					printf("the function must return a value.\n");
					return false;
				}
				
				llvm_builder.CreateRetVoid();
				return true;
			}
			
			auto expr = this->encode_expr(node->value);
			if(expr == nullptr)
			{
				printf("invalid ret value.\n");
				return false;
			}
			auto value = model.get_value(llvm_builder, expr->value);
			auto actureRetType = expr->type;
			if(actureRetType != expectedRetType && !actureRetType->canLosslesslyBitCastTo(expectedRetType))
			{
				printf("the type of return value can't cast to return type of function.\n");
				return false;
			}
			
			llvm_builder.CreateRet(value);
			
			return true;
		}
		
		bool encode_stmt_if(struct ast_stmt_if_t* node)
		{
			if(node == nullptr)
				return false;
			
			llvm::BasicBlock* if_begin = llvm::BasicBlock::Create(llvm_context, "if.begin", this->func);
			llvm::BasicBlock* if_true = llvm::BasicBlock::Create(llvm_context, "if.true", this->func);
			llvm::BasicBlock* if_false = llvm::BasicBlock::Create(llvm_context, "if.false", this->func);
			llvm::BasicBlock* if_end = llvm::BasicBlock::Create(llvm_context, "if.end", this->func);
			
			llvm_builder.CreateBr(if_begin);
			llvm_builder.SetInsertPoint(if_begin);
			auto condE = this->encode_expr(node->cond);
			if(condE == nullptr)
				return false;
			auto condV = model.get_value(llvm_builder, condE->value);
			if(!condV->getType()->isIntegerTy(1))
			{
				printf("if.cond need a bool value.\n");
				return false;
			}
			llvm_builder.CreateCondBr(condV, if_true, if_false);
			
			llvm_builder.SetInsertPoint(if_true);
			if(!this->encode_stmt(node->branch_true))
				return false;
			llvm_builder.CreateBr(if_end);
			
			if(node->branch_false != nullptr)
			{
				llvm_builder.SetInsertPoint(if_false);
				if(!this->encode_stmt(node->branch_false))
					return false;
				llvm_builder.CreateBr(if_end);
			}
			
			llvm_builder.SetInsertPoint(if_end);
			
			return true;
		}
		
		bool encode_stmt_while(struct ast_stmt_while_t* node)
		{
			if(node == nullptr)
				return false;
			
			this->pushScope();
			
			llvm::BasicBlock* while_begin = llvm::BasicBlock::Create(llvm_context, "while.begin", this->func);
			llvm::BasicBlock* while_body = llvm::BasicBlock::Create(llvm_context, "while.body", this->func);
			llvm::BasicBlock* while_end = llvm::BasicBlock::Create(llvm_context, "while.end", this->func);
			
			auto oldContinuePoint = this->continuePoint;
			auto oldBreakPoint = this->breakPoint;
			
			this->continuePoint = while_begin;
			this->breakPoint = while_end;
			
			llvm_builder.CreateBr(while_begin);
			llvm_builder.SetInsertPoint(while_begin);
			auto condE = this->encode_expr(node->cond);
			if(condE == nullptr)
				return false;
			auto condV = model.get_value(llvm_builder, condE->value);
			if(!condV->getType()->isIntegerTy(1))
			{
				printf("while.cond need a bool value.\n");
				return false;
			}
			llvm_builder.CreateCondBr(condV, while_body, while_end);
			
			llvm_builder.SetInsertPoint(while_body);
			if(!this->encode_stmt(node->body))
				return false;
			llvm_builder.CreateBr(while_begin);
			
			llvm_builder.SetInsertPoint(while_end);
			
			this->continuePoint = oldContinuePoint;
			this->breakPoint = oldBreakPoint;
			
			this->popScope();
			
			return true;
		}
		
		bool encode_stmt_for(struct ast_stmt_for_t* node)
		{
			if(node == nullptr)
				return false;
			
			this->pushScope();
			
			llvm::BasicBlock* for_init = llvm::BasicBlock::Create(llvm_context, "for.init", this->func);
			llvm::BasicBlock* for_test = llvm::BasicBlock::Create(llvm_context, "for.test", this->func);
			llvm::BasicBlock* for_step = llvm::BasicBlock::Create(llvm_context, "for.step", this->func);
			llvm::BasicBlock* for_body = llvm::BasicBlock::Create(llvm_context, "for.body", this->func);
			llvm::BasicBlock* for_end = llvm::BasicBlock::Create(llvm_context, "for.end", this->func);
			
			auto oldContinuePoint = this->continuePoint;
			auto oldBreakPoint = this->breakPoint;
			this->continuePoint = for_step;
			this->breakPoint = for_end;
			
			llvm_builder.CreateBr(for_init);
			llvm_builder.SetInsertPoint(for_init);
			if(!this->encode_stmt(node->init))
				return false;
			llvm_builder.CreateBr(for_test);
			
			llvm_builder.SetInsertPoint(for_test);
			auto condE = this->encode_expr(node->cond);
			if(condE == nullptr)
				return false;
			auto condV = model.get_value(llvm_builder, condE->value);
			if(!condV->getType()->isIntegerTy(1))
			{
				printf("for.cond need a bool value.\n");
				return false;
			}
			llvm_builder.CreateCondBr(condV, for_body, for_end);
			
			llvm_builder.SetInsertPoint(for_body);
			if(!this->encode_stmt(node->body))
				return false;
			llvm_builder.CreateBr(for_step);
			
			llvm_builder.SetInsertPoint(for_step);
			if(!this->encode_stmt(node->step))
				return false;
			llvm_builder.CreateBr(for_test);
			
			llvm_builder.SetInsertPoint(for_end);
			
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
			
			auto ptr = model.ref_value(llvm_builder, leftE->value);
			auto val = model.get_value(llvm_builder, rightE->value);
			llvm_builder.CreateStore(val, ptr);
			
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
	
	llvm::Module* llvm_encode(llvm::LLVMContext& context, ast_module_t* module)
	{
		llvm_coder_t coder(context);
		llvm::Module* llvm_module = coder.encode(module);
		return llvm_module;
	}
_EndNamespace(eokas)
