#include "coder.h"
#include "object.h"
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
		llvm::LLVMContext& llvm_context;
		llvm::IRBuilder<> llvm_builder;
		llvm::Module* llvm_module;
		llvm_model_t model;
		std::vector<llvm_expr_t*> exprs;
		std::vector<llvm_type_t*> types;
		std::map<llvm::Type*, llvm_type_t*> typemappings;
		llvm_scope_t* root;
		llvm_scope_t* scope;
		llvm_func_t* func;
		llvm::BasicBlock* continuePoint;
		llvm::BasicBlock* breakPoint;
		
		llvm::Value* const_zero;
		
		explicit llvm_coder_t(llvm::LLVMContext& llvm_context) : llvm_context(llvm_context), llvm_builder(llvm_context), llvm_module(nullptr), model(llvm_context)
		{
			this->root = new llvm_scope_t(nullptr, nullptr);
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
			_DeleteList(this->types);
		}
		
		llvm_expr_t* new_expr(llvm::Value* value, llvm::Type* type = nullptr)
		{
            llvm_expr_t* expr = nullptr;
            if(value->getType()->isPointerTy() && value->getType()->getPointerElementType()->isFunctionTy())
            {
                expr = new llvm_func_t(value, type);
            }
            else
            {
                expr = new llvm_expr_t(value, type);
            }
            if(expr != nullptr)
            {
                this->exprs.push_back(expr);
            }
			return expr;
		}
		
		llvm_type_t* new_type(const String& name, llvm::Type* type)
		{
            if(type->isStructTy() || (type->isPointerTy() && type->getPointerElementType()->isStructTy()))
            {
                auto* t = new llvm_struct_t();
                t->name = name;
                t->type = type;
                t->base = nullptr;
                t->members.clear();
                this->types.push_back(t);
                return t;
            }
            else
            {
                auto* t = new llvm_type_t();
                t->name = name;
                t->type = type;
                this->types.push_back(t);
                return t;
            }
		}
		
		void pushScope(llvm::Function* f = nullptr)
		{
			this->scope = this->scope->addChild(f);
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
            
            this->scope->addType("void", this->new_type("void", model.type_void));
			this->scope->addType("i8", this->new_type("i8", model.type_i8));
			this->scope->addType("i32", this->new_type("i32", model.type_i32));
			this->scope->addType("i64", this->new_type("i64", model.type_i64));
			this->scope->addType("u8", this->new_type("u8", model.type_u8));
			this->scope->addType("u32", this->new_type("u32", model.type_u32));
			this->scope->addType("u64", this->new_type("u64", model.type_u64));
			this->scope->addType("f32", this->new_type("f32", model.type_f32));;
			this->scope->addType("f64", this->new_type("f64", model.type_f64));;
			this->scope->addType("bool", this->new_type("bool", model.type_bool));;
			this->scope->addType("string", this->new_type("string", model.type_string_ptr));
			
			this->scope->addSymbol("print", this->new_expr(model.define_func_print(llvm_module)));
			
			llvm::FunctionType* funcType = llvm::FunctionType::get(model.type_i32, false);
			llvm::Function* funcPtr = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, "main", llvm_module);
            this->func = dynamic_cast<llvm_func_t*>(this->new_expr(funcPtr));

            this->pushScope(funcPtr);
            {
                llvm::BasicBlock *entry = llvm::BasicBlock::Create(llvm_context, "entry", funcPtr);
                llvm_builder.SetInsertPoint(entry);

                for (auto &stmt: node->get_func()->body) {
                    if (!this->encode_stmt(stmt))
                        return false;
                }

                auto& lastOp = llvm_builder.GetInsertBlock()->back();
                if(!lastOp.isTerminator())
                {
                    if(funcPtr->getReturnType()->isVoidTy())
                        llvm_builder.CreateRetVoid();
                    else
                        llvm_builder.CreateRet(model.get_default_value_by_type(funcPtr->getReturnType()));
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
				printf("type node is null. \n");
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
				printf("type node is null. \n");
				return nullptr;
			}
			
			auto* elementType = this->encode_type(node->elementType);
			if(elementType == nullptr)
				return nullptr;
			
			llvm::Type* arrayType = llvm::ArrayType::get(elementType->type, node->length);
			
			return this->new_type(
				String::format("Array<%s, %d>", elementType->name.cstr(), node->length),
				arrayType
			);
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


			llvm::BasicBlock* trinary_begin = llvm::BasicBlock::Create(llvm_context, "trinary.begin", this->scope->func);
			llvm::BasicBlock* trinary_true = llvm::BasicBlock::Create(llvm_context, "trinary.true", this->scope->func);
			llvm::BasicBlock* trinary_false = llvm::BasicBlock::Create(llvm_context, "trinary.false", this->scope->func);
			llvm::BasicBlock* trinary_end = llvm::BasicBlock::Create(llvm_context, "trinary.end", this->scope->func);
			
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
					if(right->type == lhs->getType())
						return this->new_expr(llvm_builder.getTrue());
					else
						return this->new_expr(llvm_builder.getFalse());
				}
					break;
				case ast_binary_oper_t::As:
				{
					if(lhs->getType()->isSingleValueType())
					{
						if(right->type == model.type_string_ptr)
						{
							llvm::Value* value = model.value_to_string(llvm_module, this->scope->func, llvm_builder, lhs);
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
			
			auto str = model.make(llvm_module, this->scope->func, llvm_builder, model.type_string);
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
				if(argType->type->isFunctionTy() || argType->type->isStructTy() || argType->type->isArrayTy())
					argTypes.push_back(argType->type->getPointerTo());
				else
					argTypes.push_back(argType->type);
			}
			
			llvm::FunctionType* funcType = llvm::FunctionType::get(retType->type, argTypes, false);
			llvm::Function* funcPtr = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, "", llvm_module);

            u32_t index = 0;
			for (auto& arg: funcPtr->args())
			{
				const char* name = node->args[index++]->name.cstr();
				arg.setName(name);
			}
			
			auto oldFunc = this->func;
			auto oldIB = llvm_builder.GetInsertBlock();

            this->func = dynamic_cast<llvm_func_t*>(this->new_expr(funcPtr));

            this->pushScope(funcPtr);
			{
				llvm::BasicBlock* entry = llvm::BasicBlock::Create(llvm_context, "entry", funcPtr);
				llvm_builder.SetInsertPoint(entry);
				
				// self
				auto self = this->new_expr(funcPtr, funcType->getPointerTo());
                this->scope->addSymbol("self", self);
				
				// args
				for (auto& arg: funcPtr->args())
				{
					const char* name = arg.getName().data();
					auto expr = this->new_expr(&arg);
                    if(!this->scope->addSymbol(name, expr))
                    {
                        printf("The symbol name '%s' is already existed.", name);
                        return nullptr;
                    }
				}
				
				// body
                llvm::BasicBlock* body = llvm::BasicBlock::Create(llvm_context, "body", funcPtr);
                llvm_builder.SetInsertPoint(body);
				for (auto& stmt: node->body)
				{
					if(!this->encode_stmt(stmt))
						return nullptr;
				}
				auto& lastOp = llvm_builder.GetInsertBlock()->back();
				if(!lastOp.isTerminator())
				{
					if(funcPtr->getReturnType()->isVoidTy())
						llvm_builder.CreateRetVoid();
					else
						llvm_builder.CreateRet(model.get_default_value_by_type(funcPtr->getReturnType()));
				}

                // up-value
                llvm_builder.SetInsertPoint(entry);
                for (auto& upval : this->func->upvals.table)
                {
                    const String& name = upval.first;
                    llvm_expr_t* expr = upval.second;

                    llvm::Value* val = model.get_value(llvm_builder, expr->value);
                    llvm::Value* ptr = llvm_builder.CreateAlloca(val->getType());
                    llvm_builder.CreateStore(val, ptr);
                    if(!this->scope->addSymbol(name, this->new_expr(val)))
                    {
                        printf("The symbol name '%s' is already existed.", name.cstr());
                        return nullptr;
                    }
                }
                llvm_builder.CreateBr(body);
			}
			this->popScope();

			this->func = oldFunc;
			llvm_builder.SetInsertPoint(oldIB);
			
			return this->new_expr(funcPtr);
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
				printf("invalid function type.");
				return nullptr;
			}
			
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
			
			std::vector<llvm::Constant*> arrayElements;
			llvm::Type* elementType = nullptr;
			for (auto element: node->elements)
			{
				auto elementE = this->encode_expr(element);
				if(elementE == nullptr)
					return nullptr;
				auto elementV = llvm::cast<llvm::Constant>(elementE->value);
				if(elementV == nullptr)
					return nullptr;
				
				auto thisItemType = elementV->getType();
				if(elementType == nullptr)
				{
					elementType = thisItemType;
				}
				else if(thisItemType != elementType)
				{
					printf("the type of some elements is not same as others.");
					return nullptr;
				}
				
				arrayElements.push_back(elementV);
			}
			
			if(arrayElements.empty() || elementType == nullptr)
				elementType = llvm::Type::getInt32Ty(llvm_context);
			
			auto arrayType = llvm::ArrayType::get(elementType, node->elements.size());
			auto arrayValue = llvm::ConstantArray::get(arrayType, arrayElements);
			auto arrayPtr = model.make(llvm_module, this->scope->func, llvm_builder, arrayType);
			llvm_builder.CreateStore(arrayValue, arrayPtr);
			
			return this->new_expr(arrayPtr);
		}
		
		llvm_expr_t* encode_expr_index_ref(struct ast_expr_index_ref_t* node)
		{
			if(node == nullptr)
				return nullptr;
			
			auto objE = this->encode_expr(node->obj);
			auto keyE = this->encode_expr(node->key);
			if(objE == nullptr || keyE == nullptr)
				return nullptr;
			
			auto objV = model.get_value(llvm_builder, objE->value);
			auto keyV = model.get_value(llvm_builder, keyE->value);
			if(!objV->getType()->isPointerTy() || !objV->getType()->getPointerElementType()->isArrayTy())
			{
				printf("index-access is not defined on the object.");
				return nullptr;
			}
			if(!keyV->getType()->isIntegerTy())
			{
				printf("the type of index is invalid.");
				return nullptr;
			}
			
			llvm::Value* ptr = llvm_builder.CreateGEP(objV, {llvm_builder.getInt32(0), keyV});
			return this->new_expr(ptr);
		}
		
		llvm_expr_t* encode_expr_object_def(struct ast_expr_object_def_t* node)
		{
			if(node == nullptr)
				return nullptr;
			
			auto* structTypeDef = dynamic_cast<llvm_struct_t*>(this->encode_type(node->type));
			if(structTypeDef == nullptr)
				return nullptr;

			for (auto& objectMember: node->members)
			{
				auto structMember = structTypeDef->getMember(objectMember.first);
				if(structMember == nullptr)
				{
					printf("object member '%s' is not defined in struct.", objectMember.first.cstr());
					return nullptr;
				}
				// todo: check object-member-type equals to struct-member-type.
			}
			
			// make object
			auto structType = structTypeDef->type;
			llvm::Value* objectValue = model.make(llvm_module, this->scope->func, llvm_builder, structType);
			
			u32_t index = -1;
			for (auto& structMember: structTypeDef->members)
			{
				index += 1;
				auto objectMember = node->members.find(structMember->name);
				llvm::Value* memV = nullptr;
				if(objectMember != node->members.end())
				{
					auto memE = this->encode_expr(objectMember->second);
					if(memE == nullptr)
						return nullptr;
					memV = model.get_value(llvm_builder, memE->value);
				}
				else
				{
					auto memT = structMember->type->type;
					memV = model.get_default_value_by_type(memT);
				}
				
				if(memV)
				{
					llvm::Value* memPtr = llvm_builder.CreateStructGEP(structType, objectValue, index, structMember->name.cstr());
					llvm_builder.CreateStore(memV, memPtr);
				}
			}
			
			return this->new_expr(objectValue);
		}
		
		llvm_expr_t* encode_expr_object_ref(struct ast_expr_object_ref_t* node)
		{
			if(node == nullptr)
				return nullptr;
			
			auto objectE = this->encode_expr(node->obj);
			auto key = llvm_builder.CreateGlobalString(node->key.cstr());
			if(objectE == nullptr || key == nullptr)
				return nullptr;
			
			auto objectV = model.get_value(llvm_builder, objectE->value);
			auto objectT = objectV->getType();
			if(!(objectT->isPointerTy() && objectT->getPointerElementType()->isStructTy()))
			{
				printf("the value is not a object reference.");
				return nullptr;
			}
			auto structTypeIter = this->typemappings.find(objectT->getPointerElementType());
			if(structTypeIter == this->typemappings.end())
			{
				printf("can not find the type of this value.");
				return nullptr;
			}
			auto structTypeDef = dynamic_cast<llvm_struct_t*>(structTypeIter->second);
			auto index = structTypeDef->getMemberIndex(node->key);
			if(index == -1)
			{
				printf("can not get the index of object member.");
				return nullptr;
			}

			auto structType = structTypeDef->type;
			llvm::Value* value = llvm_builder.CreateStructGEP(structType, objectV, index);
			return this->new_expr(value);
		}
		
		bool encode_stmt(struct ast_stmt_t* node)
		{
			if(node == nullptr)
				return false;
			
			switch (node->category)
			{
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
		
		bool encode_stmt_struct_def(struct ast_stmt_struct_def_t* node)
		{
			if(node == nullptr)
				return false;
			
			const String& name = node->name;

			auto* structTypeDef = dynamic_cast<llvm_struct_t*>(this->new_type(name, nullptr));
			
			if(node->base != nullptr)
			{
				auto baseType = dynamic_cast<llvm_struct_t*>(this->encode_type(node->base));
				if(baseType == nullptr)
					return false;
				for(const auto& baseMember: baseType->members)
				{
					auto& mem = baseMember;
					if(structTypeDef->getMember(mem->name) != nullptr)
					{
						printf("member named '%s' is already exists", mem->name.cstr());
						return false;
					}
					structTypeDef->addMember(mem->name, mem->type);
				}
			}
			
			for (const auto& thisMember: node->members)
			{
				auto& mem = thisMember.second;
				
				const auto& memName = mem->name;
				if(structTypeDef->getMember(memName) != nullptr)
				{
					printf("member named '%s' is already exists", memName.cstr());
					return false;
				}
				
				const auto& memType = this->encode_type(mem->type);
				if(memType == nullptr)
					return false;
				
				structTypeDef->addMember(memName, memType);
			}
			
			std::vector<llvm::Type*> memberTypes;
			for(auto& mem : structTypeDef->members)
			{
				memberTypes.push_back(mem->type->type);
			}
			String structName = String::format("Type.%s.Struct", name.cstr());
			auto* structType = llvm::StructType::create(llvm_context);
			structType->setName(structName.cstr());
			structType->setBody(memberTypes);
			structTypeDef->type = structType;
			
			if(!this->scope->addType(name, structTypeDef))
            {
                printf("There is a same type named %s in this scope.", name.cstr());
                return false;
            }
			this->typemappings[structTypeDef->type] = structTypeDef;

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
				argTypes.push_back(argType->type);
			}
			
			llvm::FunctionType* procType = llvm::FunctionType::get(retType->type, argTypes, false);
			llvm::Type* procPtrType = procType->getPointerTo();

			this->scope->addType(node->name, this->new_type(node->name, procPtrType));
			
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
			
			llvm::Type* stype = nullptr;
			llvm::Type* vtype = value->getType();
			if(type != nullptr)
			{
				stype = type->type;
				do
				{
					if(stype == vtype)
						break;
					if(vtype->canLosslesslyBitCastTo(stype))
						break;
					if(vtype->isPointerTy() && stype == vtype->getPointerElementType())
					{
						stype = type->type = vtype;
						break;
					}
					
					printf("type of value can not cast to the type of symbol.\n");
					return false;
				}
				while(false);
			}
			else
			{
				stype =  vtype;
			}
			
			if(stype->isVoidTy())
			{
				printf("void type can't assign to a symbol.");
				return false;
			}
			// TODO: 校验类型合法性, 值类型是否遵循标记类型
			
			llvm::Value* symbol = llvm_builder.CreateAlloca(stype);
			llvm_builder.CreateStore(value, symbol);
			symbol = model.ref_value(llvm_builder, symbol);
			symbol->setName(node->name.cstr());
			
			auto symbolE = this->new_expr(symbol, expr->type);
			if(!scope->addSymbol(node->name, symbolE))
            {
                printf("There is a symbol named %s in this scope.", node->name.cstr());
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
			
			auto expectedRetType = this->scope->func->getFunctionType()->getReturnType();
			
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
			auto actureRetType = value->getType();
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
			
			llvm::BasicBlock* if_begin = llvm::BasicBlock::Create(llvm_context, "if.begin", this->scope->func);
			llvm::BasicBlock* if_true = llvm::BasicBlock::Create(llvm_context, "if.true", this->scope->func);
			llvm::BasicBlock* if_false = llvm::BasicBlock::Create(llvm_context, "if.false", this->scope->func);
			llvm::BasicBlock* if_end = llvm::BasicBlock::Create(llvm_context, "if.end", this->scope->func);
			
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
			auto& lastOp = if_true->back();
			if(!lastOp.isTerminator())
			{
				llvm_builder.CreateBr(if_end);
			}
			
			if(node->branch_false != nullptr)
			{
				llvm_builder.SetInsertPoint(if_false);
				if(!this->encode_stmt(node->branch_false))
					return false;
				auto& lastOp = if_false->back();
				if(!lastOp.isTerminator())
				{
					llvm_builder.CreateBr(if_end);
				}
			}
			
			llvm_builder.SetInsertPoint(if_end);
			
			return true;
		}
		
		bool encode_stmt_while(struct ast_stmt_while_t* node)
		{
			if(node == nullptr)
				return false;
			
			this->pushScope();
			
			llvm::BasicBlock* while_begin = llvm::BasicBlock::Create(llvm_context, "while.begin", this->scope->func);
			llvm::BasicBlock* while_body = llvm::BasicBlock::Create(llvm_context, "while.body", this->scope->func);
			llvm::BasicBlock* while_end = llvm::BasicBlock::Create(llvm_context, "while.end", this->scope->func);
			
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
			auto& lastOp = while_body->back();
			if(!lastOp.isTerminator())
			{
				llvm_builder.CreateBr(while_begin);
			}
			
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
			
			llvm::BasicBlock* for_init = llvm::BasicBlock::Create(llvm_context, "for.init", this->scope->func);
			llvm::BasicBlock* for_test = llvm::BasicBlock::Create(llvm_context, "for.test", this->scope->func);
			llvm::BasicBlock* for_step = llvm::BasicBlock::Create(llvm_context, "for.step", this->scope->func);
			llvm::BasicBlock* for_body = llvm::BasicBlock::Create(llvm_context, "for.body", this->scope->func);
			llvm::BasicBlock* for_end = llvm::BasicBlock::Create(llvm_context, "for.end", this->scope->func);
			
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
			auto& lastOp = for_body->back();
			if(!lastOp.isTerminator())
			{
				llvm_builder.CreateBr(for_step);
			}
			
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
			if(leftE == nullptr || rightE == nullptr)
				return false;
			
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
	
	llvm::Module* llvm_encode_test(llvm::LLVMContext& context, ast_module_t* ast)
	{
		llvm_model_t model(context);
		auto* module = new llvm::Module("test", context);
		model.declare_cfunc_sprintf(module);
		model.declare_cfunc_puts(module);
		model.declare_cfunc_printf(module);
		model.declare_cfunc_malloc(module);
		model.declare_cfunc_free(module);

		// test
		llvm::FunctionType* testT = llvm::FunctionType::get(model.type_i32, {model.type_i32}, false);
		auto testV = llvm::Function::Create(testT, llvm::Function::ExternalLinkage, "test", module);
		{
			llvm::IRBuilder<> builder(context);
			llvm::BasicBlock* entry = llvm::BasicBlock::Create(context, "entry", testV);
			builder.SetInsertPoint(entry);
			
			auto arg0 = testV->getArg(0);
			
			llvm::BasicBlock* if_true = llvm::BasicBlock::Create(context, "if.true", testV);
			llvm::BasicBlock* if_false = llvm::BasicBlock::Create(context, "if.false", testV);

			auto cond = builder.CreateICmpSLE(arg0, builder.getInt32(0));
			builder.CreateCondBr(cond, if_true, if_false);
			
			builder.SetInsertPoint(if_true);
			builder.CreateRet(builder.getInt32(1));
			
			builder.SetInsertPoint(if_false);
			auto _0 = builder.CreateSub(arg0, builder.getInt32(1));
			auto _1 = builder.CreateCall(testV, {_0});
			auto _2 = builder.CreateMul(arg0, _1);
			builder.CreateRet(_2);
		}
		
		// main
		llvm::FunctionType* mainT = llvm::FunctionType::get(model.type_i32, false);
		auto mainV = llvm::Function::Create(mainT, llvm::Function::ExternalLinkage, "main", module);
		{
			llvm::IRBuilder<> builder(context);
			llvm::BasicBlock* entry = llvm::BasicBlock::Create(context, "entry", mainV);
			builder.SetInsertPoint(entry);
			
			auto ret = builder.CreateCall(testV, builder.getInt32(10));
			
			model.print(module, mainV, builder, {ret});
			builder.CreateRet(builder.getInt32(0));
		}
		
		return module;
	}
_EndNamespace(eokas)
