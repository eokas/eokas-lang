#include "./x-module-coder.h"
#include "./model.h"

namespace eokas {
    omis_module_coder_t::omis_module_coder_t(omis_context_t* context, const String &name)
            : omis_module_t(context, name) {
    }

    bool omis_module_coder_t::encode_module(ast_node_module_t *node) {
        if (node == nullptr)
            return false;

        // Imports
        this->using_module("core");
        for(auto& imp : node->imports) {
            if(!this->using_module(imp.second->target)) {
                return false;
            }
        }

        // Body
        omis_type_t *ret = type_i32();
        std::vector<omis_type_t *> args = {};
        omis_value_t *func = this->value_func("$main", ret, args, false);
        this->scope->add_value_symbol("$main", func);
        this->scope->func = func;
        this->push_scope(func);
        {
            auto entry = this->create_block("entry");
            // IR.SetInsertPoint(entry);
            this->set_active_block(entry);

            for (auto& stmt: node->entry->body) {
                if (!this->encode_stmt(stmt))
                    return false;
            }

            this->stmt_ensure_tail_ret(func);
        }
        this->pop_scope();

        // Exports
        for(auto& exp : node->exports) {
            omis_type_symbol_t* symbol = this->get_type_symbol(exp.second->name, true);
            if(symbol == nullptr) {
                return false;
            }
            this->exports->add_type_symbol(symbol->name, symbol->type);
        }

        return true;
    }

    bool omis_module_coder_t::encode_import(ast_node_import_t* node) {
        if(node == nullptr)
            return false;
        return this->using_module(node->target);
    }

    bool omis_module_coder_t::encode_export(ast_node_export_t* node) {
        if(node == nullptr)
            return false;
        omis_type_symbol_t* symbol = this->get_type_symbol(name, true);
        if(symbol == nullptr) {
            printf("The symbol named '%s' is undefined.", node->name.cstr());
            return false;
        }
        return this->exports->add_type_symbol(node->name, symbol->type);
    }

    bool omis_module_coder_t::encode_stmt(ast_node_stmt_t *node) {
        if (node == nullptr)
            return false;

        switch (node->category) {
            case ast_category_t::BLOCK:
                return this->encode_stmt_block(dynamic_cast<ast_node_block_t *>(node));
            case ast_category_t::SYMBOL_DEF:
                return this->encode_stmt_symbol_def(dynamic_cast<ast_node_symbol_def_t *>(node));
            case ast_category_t::ASSIGN:
                return this->encode_stmt_assign(dynamic_cast<ast_node_assign_t *>(node));
            case ast_category_t::RETURN:
                return this->encode_stmt_return(dynamic_cast<ast_node_return_t *>(node));
            case ast_category_t::IF:
                return this->encode_stmt_if(dynamic_cast<ast_node_if_t *>(node));
            case ast_category_t::LOOP:
                return this->encode_stmt_loop(dynamic_cast<ast_node_loop_t *>(node));
            case ast_category_t::BREAK:
                return this->encode_stmt_break(dynamic_cast<ast_node_break_t *>(node));
            case ast_category_t::CONTINUE:
                return this->encode_stmt_continue(dynamic_cast<ast_node_continue_t *>(node));
            default:
                return false;
        }

        return false;
    }

    bool omis_module_coder_t::encode_stmt_block(ast_node_block_t *node) {
        if (node == nullptr)
            return false;

        return this->stmt_block([&]()->bool{
            for (auto &stmt: node->stmts) {
                if (!this->encode_stmt(stmt))
                    return false;
            }
            return true;
        });
    }

    bool omis_module_coder_t::encode_stmt_symbol_def(ast_node_symbol_def_t *node) {
        if (node == nullptr)
            return false;
		
        std::optional<omis_lambda_type_t> lambda_type;
        if(node->type != nullptr) {
            lambda_type = [&]() -> omis_type_t * {
                return this->encode_type_ref(node->type);
            };
        }

        auto lambda_value = [&]()->omis_value_t* {
            return this->encode_expr(node->value);
        };

        return this->stmt_symbol_def(node->name, lambda_type, lambda_value);
    }

    bool omis_module_coder_t::encode_stmt_assign(ast_node_assign_t *node) {
        if (node == nullptr)
            return false;
		
        auto left = [&]()->omis_value_t* {
            return this->encode_expr(node->left);
        };

        auto right = [&]()->omis_value_t* {
            return this->encode_expr(node->right);
        };

        return this->stmt_assign(left, right);
    }

    bool omis_module_coder_t::encode_stmt_return(ast_node_return_t *node) {
        if (node == nullptr)
            return false;
		
        std::optional<omis_lambda_expr_t> value;
        if(node->value != nullptr) {
            value = [&]()->omis_value_t* {
                return this->encode_expr(node->value);
            };
        }

        return this->stmt_return(value);
    }

    bool omis_module_coder_t::encode_stmt_if(struct ast_node_if_t *node) {
		if (node == nullptr)
			return false;
		
        auto cond = [&]()->auto {
            return this->encode_expr(node->cond);
        };

        auto branch_true = [&]()->bool {
            if(node->branch_true == nullptr)
                return true;
            return this->encode_stmt(node->branch_true);
        };

        auto branch_false = [&]()->bool {
            if(node->branch_false == nullptr)
                return true;
            return this->encode_stmt(node->branch_false);
        };

		return this->stmt_branch(cond, branch_true, branch_false);
	}

    bool omis_module_coder_t::encode_stmt_loop(ast_node_loop_t *node) {
		if (node == nullptr)
			return false;
		
        auto init = [&]() -> auto {
            if (node->init == nullptr)
                return true;
            return this->encode_stmt(node->init);
        };

        auto cond = [&]() -> auto {
            if (node->cond == nullptr)
                return this->value_bool(true);
            return this->encode_expr(node->cond);
        };

        auto step = [&]() -> auto {
            if (node->step == nullptr)
                return true;
            return this->encode_stmt(node->step);
        };

        auto body = [&]() -> auto {
            if (node->body == nullptr)
                return true;
            return this->encode_stmt(node->body);
        };

		return this->stmt_loop(init, cond, step, body);
	}

    bool omis_module_coder_t::encode_stmt_break(ast_node_break_t *node) {
        if (node == nullptr)
            return false;
        return this->stmt_break();
    }

    bool omis_module_coder_t::encode_stmt_continue(ast_node_continue_t *node) {
        if (node == nullptr)
            return false;
        return this->stmt_continue();
    }

    omis_type_t *omis_module_coder_t::encode_type_ref(ast_node_type_t *node) {
        if (node == nullptr) {
            printf("ERROR: Type node is null. \n");
            return nullptr;
        }

        const String &name = node->name;

        auto *symbol = this->scope->get_type_symbol(name, true);
        if (symbol == nullptr) {
            printf("ERROR: The type '%s' is undefined.\n", name.cstr());
            return nullptr;
        }

        return symbol->type;
    }

    omis_value_t *omis_module_coder_t::encode_expr(ast_node_expr_t *node) {
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
                /*
                case ast_category_t::ARRAY_DEF:
                    return this->encode_expr_array_def(dynamic_cast<ast_node_array_def_t *>(node));
                case ast_category_t::ARRAY_REF:
                    return this->encode_expr_index_ref(dynamic_cast<ast_node_array_ref_t *>(node));
                case ast_category_t::OBJECT_DEF:
                    return this->encode_expr_object_def(dynamic_cast<ast_node_object_def_t *>(node));
                case ast_category_t::OBJECT_REF:
                    return this->encode_expr_object_ref(dynamic_cast<ast_node_object_ref_t *>(node));
                     */
            default:
                return nullptr;
        }
        return nullptr;
    }

    omis_value_t *omis_module_coder_t::encode_expr_trinary(struct ast_node_expr_trinary_t *node) {
        if (node == nullptr)
            return nullptr;
		
		auto lambda_cond = [&]()->omis_value_t* {
			return this->encode_expr(node->cond);
		};
		auto lambda_true = [&]()->omis_value_t* {
			return this->encode_expr(node->branch_true);
		};
		auto lambda_false = [&]()->omis_value_t* {
			return this->encode_expr(node->branch_false);
		};
		
		return this->expr_branch(lambda_cond, lambda_true, lambda_false);
    }

    omis_value_t *omis_module_coder_t::encode_expr_binary(ast_node_expr_binary_t *node) {
        if (node == nullptr)
            return nullptr;

        auto func = this->scope->func;

        auto lhs = this->encode_expr(node->left);
        auto rhs = this->encode_expr(node->right);
        if (lhs == nullptr || rhs == nullptr)
            return nullptr;

        lhs = this->get_ptr_val(lhs);
        rhs = this->get_ptr_val(rhs);

        switch (node->op) {
            case ast_binary_oper_t::OR:
                return this->l_or(lhs, rhs);
            case ast_binary_oper_t::AND:
                return this->l_and(lhs, rhs);
            case ast_binary_oper_t::EQ:
                return this->eq(lhs, rhs);
            case ast_binary_oper_t::NE:
                return this->ne(lhs, rhs);
            case ast_binary_oper_t::LE:
                return this->le(lhs, rhs);
            case ast_binary_oper_t::GE:
                return this->ge(lhs, rhs);
            case ast_binary_oper_t::LT:
                return this->lt(lhs, rhs);
            case ast_binary_oper_t::GT:
                return this->gt(lhs, rhs);
            case ast_binary_oper_t::ADD:
                return this->add(lhs, rhs);
            case ast_binary_oper_t::SUB:
                return this->sub(lhs, rhs);
            case ast_binary_oper_t::MUL:
                return this->mul(lhs, rhs);
            case ast_binary_oper_t::DIV:
                return this->div(lhs, rhs);
            case ast_binary_oper_t::MOD:
                return this->mod(lhs, rhs);
            case ast_binary_oper_t::BIT_AND:
                return this->b_and(lhs, rhs);
            case ast_binary_oper_t::BIT_OR:
                return this->b_or(lhs, rhs);
            case ast_binary_oper_t::BIT_XOR:
                return this->b_xor(lhs, rhs);
            case ast_binary_oper_t::SHIFT_L:
                return this->b_shl(lhs, rhs);
            case ast_binary_oper_t::SHIFT_R:
                return this->b_shr(lhs, rhs);
            default:
                return nullptr;
        }
    }

    omis_value_t *omis_module_coder_t::encode_expr_unary(ast_node_expr_unary_t *node) {
        if (node == nullptr)
            return nullptr;

        auto rhs = this->encode_expr(node->right);
        if (rhs == nullptr)
            return nullptr;

        rhs = this->get_ptr_val(rhs);

        switch (node->op) {
            case ast_unary_oper_t::POS:
                return rhs;
            case ast_unary_oper_t::NEG:
                return this->neg(rhs);
            case ast_unary_oper_t::NOT:
                return this->l_not(rhs);
            case ast_unary_oper_t::FLIP:
                return this->b_flip(rhs);
            default:
                return nullptr;
        }
    }

    omis_value_t *omis_module_coder_t::encode_expr_int(ast_node_literal_int_t *node) {
        if (node == nullptr)
            return nullptr;

        u64_t vals = *((u64_t *) &(node->value));
        u32_t bits = vals > 0xFFFFFFFF ? 64 : 32;

        return this->value_integer(vals, bits);
    }

    omis_value_t *omis_module_coder_t::encode_expr_float(ast_node_literal_float_t *node) {
        if (node == nullptr)
            return nullptr;

        return this->value_float(node->value);
    }

    omis_value_t *omis_module_coder_t::encode_expr_bool(ast_node_literal_bool_t *node) {
        if (node == nullptr)
            return nullptr;
        return this->value_bool(node->value);
    }

    omis_value_t *omis_module_coder_t::encode_expr_string(ast_node_literal_string_t *node) {
        if (node == nullptr)
            return nullptr;

        // auto str = func->string_make(node->value.cstr());
        // return str;

        return nullptr;
    }

    omis_value_t *omis_module_coder_t::encode_expr_symbol_ref(ast_node_symbol_ref_t *node) {
        if (node == nullptr)
            return nullptr;

        auto *symbol = this->scope->get_value_symbol(node->name, true);
        if (symbol == nullptr) {
            printf("ERROR: Symbol '%s' is undefined.\n", node->name.cstr());
            return nullptr;
        }

        // local-value-ref
        if (symbol->scope->func == this->scope->func) {
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

    omis_value_t *omis_module_coder_t::encode_expr_func_def(ast_node_func_def_t *node) {
        if (node == nullptr)
            return nullptr;
		
        auto *ret_type = this->encode_type_ref(node->rtype);
        if (ret_type == nullptr)
            return nullptr;

        std::vector<omis_type_t *> args_types;
        for (auto &arg: node->args) {
            auto *arg_type = this->encode_type_ref(arg.type);
            if (arg_type == nullptr)
                return nullptr;
            if (arg_type->is_type_func() || arg_type->is_type_array() || arg_type->is_type_struct())
                args_types.push_back(arg_type->get_pointer_type());
            else
                args_types.push_back(arg_type);
        }

        auto newFunc = this->value_func("", ret_type, args_types, false);

        this->push_scope(newFunc);
        {
            auto *entry = this->create_block("entry");
            this->set_active_block(entry);

            // self
            auto self = newFunc;
            this->scope->add_value_symbol("self", self);

            // args
            for (size_t index = 0; index < node->args.size(); index++) {
                const char *name = node->args.at(index).name.cstr();
                auto arg = this->get_func_arg_value(newFunc, index + 1);
                arg->set_name(name);
                if (!this->scope->add_value_symbol(name, arg)) {
                    printf("ERROR: The symbol name '%s' is already existed.\n", name);
                    return nullptr;
                }
            }

            // body
            for (auto &stmt: node->body) {
                if (!this->encode_stmt(stmt))
                    return nullptr;
            }
			
            this->stmt_ensure_tail_ret(newFunc);
        }
        this->pop_scope();

        return newFunc;
    }

    omis_value_t *omis_module_coder_t::encode_expr_func_ref(ast_node_func_ref_t *node) {
        if (node == nullptr)
            return nullptr;
		
        auto expr = this->encode_expr(node->func);
        if (expr == nullptr) {
            printf("The function is undefined.\n");
            return nullptr;
        }
        auto func = this->get_ptr_val(expr);

        /*
        omis_type_t* funcType = nullptr;
        if (funcExpr->get_type()->is_type_func()) {
            funcType = llvm::cast<llvm::FunctionType>(funcPtr->getType());
        }
        else if (funcExpr->getType()->isPointerTy() &&
            funcPtr->getType()->getPointerElementType()->isFunctionTy()) {
            funcType = llvm::cast<llvm::FunctionType>(funcPtr->getType()->getPointerElementType());
        }
        else {
            printf("ERROR: Invalid function type.\n");
            return nullptr;
        }
        */

        std::vector<omis_value_t *> args;
        for (auto i = 0; i < node->args.size(); i++) {
            auto *argT = this->get_func_arg_type(func, i);
            auto *argV = this->encode_expr(node->args.at(i));
            if (argV == nullptr)
                return nullptr;
            argV = this->get_ptr_val(argV);

            if (argT != argV->get_type()) {
                if (equals_type(argT, argV->get_type())) {
                    /*
                    if (argT == this->type_cstr) {
                        argV = func->cstr_from_value(argV);
                    }
                    if (argT == this->type_string_ptr) {
                        argV = func->string_from_value(argV);
                    }
                    else if(this->can_losslessly_bitcast(argV->get_type(), argT)) {
                        printf("ERROR: The type of param[%d] can't cast to the param type of function.\n", i);
                        return nullptr;
                    }
                    */
                }

                args.push_back(argV);
            }

            auto retval = this->call(func, args);

            return retval;
        }
    }
}
