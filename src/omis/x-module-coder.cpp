#include "./x-module-coder.h"
#include "./model.h"

namespace eokas {
    omis_module_coder_t::omis_module_coder_t(omis_bridge_t* bridge, const String& name)
        : omis_module_t(bridge, name){

    }

    bool omis_module_coder_t::encode_module(ast_node_module_t* node) {
        if (node == nullptr)
            return false;

        omis_type_t* ret = type_i32();
        std::vector<omis_type_t*> args = {};
        omis_func_t* func = this->value_func("$main", ret, args, false);
        this->scope->add_value_symbol("$main", func);
        this->scope->func = func;

        this->push_scope(func);
        {
            auto entry = func->create_block("entry");
            // IR.SetInsertPoint(entry);
            func->set_active_block(entry);

            for (auto& stmt: node->entry->body) {
                if (!this->encode_stmt(stmt))
                    return false;
            }

            func->ensure_tail_ret();
        }
        this->pop_scope();

        return true;
    }

    bool omis_module_coder_t::encode_stmt(ast_node_stmt_t* node) {
        if (node == nullptr)
            return false;

        switch (node->category) {
            case ast_category_t::SYMBOL_DEF:
                return this->encode_stmt_symbol_def(dynamic_cast<ast_node_symbol_def_t*>(node));
            case ast_category_t::RETURN:
                return this->encode_stmt_return(dynamic_cast<ast_node_return_t*>(node));
            default:
                return false;
        }

        return false;
    }

    bool omis_module_coder_t::encode_stmt_symbol_def(ast_node_symbol_def_t* node) {
        if (node == nullptr)
            return false;

        auto exists = this->scope->get_value_symbol(node->name, false);
        if (exists != nullptr) {
            printf("ERROR: The symbol '%s' is aready defined.", node->name.cstr());
            return false;
        }

        auto type = node->type != nullptr ? this->encode_type_ref(node->type) : nullptr;
        auto expr = this->encode_expr(node->value);
        if (expr == nullptr)
            return false;

        omis_type_t* stype = nullptr;
        omis_type_t* dtype = type;
        omis_type_t* vtype = expr->get_type();
        if (dtype != nullptr) {
            stype = dtype;

            // Check type compatibilities.
            do {
                if (stype == vtype)
                    break;
                if (this->can_losslessly_bitcast(vtype, stype))
                    break;
                /*
                if (vtype->isPointerTy() && vtype->getPointerElementType() == stype) {
                    stype = dtype = vtype;
                    break;
                }
                */

                // TODO: 校验类型合法性, 值类型是否遵循标记类型

                printf("ERROR: Type of value can not cast to the type of symbol.\n");
                return false;
            } while (false);
        } else {
            stype = vtype;
        }

        if (this->equals_type(stype, this->type_void())) {
            printf("ERROR: Void-Type can't assign to a symbol.\n");
            return false;
        }

        if (!scope->add_value_symbol(node->name, expr)) {
            printf("ERROR: There is a symbol named %s in this scope.\n", node->name.cstr());
            return false;
        }

        return true;
    }

    bool omis_module_coder_t::encode_stmt_return(ast_node_return_t* node) {
        if (node == nullptr)
            return false;

        auto func = this->scope->func;
        auto expectedRetType = func->get_ret_type();

        if (node->value == nullptr) {
            if (this->equals_type(expectedRetType, type_void())) {
                printf("ERROR: The function must return a value.\n");
                return false;
            }

            func->ret();
            return true;
        }

        auto expr = this->encode_expr(node->value);
        if (expr == nullptr) {
            printf("ERROR: Invalid ret value.\n");
            return false;
        }

        auto actureRetType = expr->get_type();
        if (!this->equals_type(actureRetType, expectedRetType) &&
            !this->can_losslessly_bitcast(actureRetType, expectedRetType)) {
            printf("ERROR: The type of return value can't cast to return type of function.\n");
            return false;
        }

        func->ret(expr);

        return true;
    }

    omis_type_t* omis_module_coder_t::encode_type_ref(ast_node_type_t* node) {
        if (node == nullptr) {
            printf("ERROR: Type node is null. \n");
            return nullptr;
        }

        const String& name = node->name;

        auto* symbol = this->scope->get_type_symbol(name, true);
        if (symbol == nullptr) {
            printf("ERROR: The type '%s' is undefined.\n", name.cstr());
            return nullptr;
        }

        return symbol->type;
    }

    omis_value_t* omis_module_coder_t::encode_expr(ast_node_expr_t* node) {
        if (node == nullptr)
            return nullptr;

        switch (node->category) {
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
                /*
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
                 */
            default:
                return nullptr;
        }
        return nullptr;
    }

    omis_value_t* omis_module_coder_t::encode_expr_trinary(struct ast_node_expr_trinary_t* node) {
        if (node == nullptr)
            return nullptr;

        omis_func_t* func = this->scope->func;

        auto trinary_begin = func->create_block("trinary.begin");
        auto trinary_true = func->create_block("trinary.true");
        auto trinary_false = func->create_block("trinary.false");
        auto trinary_end = func->create_block("trinary.end");

        func->jump(trinary_begin);
        func->set_active_block(trinary_begin);
        auto* cond = this->encode_expr(node->cond);
        if (cond == nullptr)
            return nullptr;

        //if (!cond->getType()->isIntegerTy(1)) {
        if (!this->equals_type(cond->get_type(), this->type_i32())) {
            printf("ERROR: Condition must be a bool value.\n");
            return nullptr;
        }

        func->jump_cond(cond, trinary_true, trinary_false);

        func->set_active_block(trinary_true);
        auto* trueV = this->encode_expr(node->branch_true);
        if (trueV == nullptr)
            return nullptr;
        func->jump(trinary_end);

        func->set_active_block(trinary_false);
        auto falseV = this->encode_expr(node->branch_false);
        if (falseV == nullptr)
            return nullptr;
        func->jump(trinary_end);

        func->set_active_block(trinary_end);
        if (!this->equals_type(trueV->get_type(), falseV->get_type())) {
            printf("ERROR: Type of true-branch must be the same as false-branch.\n");
            return nullptr;
        }

        auto phi = func->phi(trueV->get_type(), {
                {trueV,  trinary_true},
                {falseV, trinary_false}
        });

        return phi;
    }

    omis_value_t* omis_module_coder_t::encode_expr_binary(ast_node_expr_binary_t* node) {
        if (node == nullptr)
            return nullptr;

        auto func = this->scope->func;

        auto lhs = this->encode_expr(node->left);
        auto rhs = this->encode_expr(node->right);
        if (lhs == nullptr || rhs == nullptr)
            return nullptr;

        switch (node->op) {
            case ast_binary_oper_t::OR:
                return func->l_or(lhs, rhs);
            case ast_binary_oper_t::AND:
                return func->l_and(lhs, rhs);
            case ast_binary_oper_t::EQ:
                return func->eq(lhs, rhs);
            case ast_binary_oper_t::NE:
                return func->ne(lhs, rhs);
            case ast_binary_oper_t::LE:
                return func->le(lhs, rhs);
            case ast_binary_oper_t::GE:
                return func->ge(lhs, rhs);
            case ast_binary_oper_t::LT:
                return func->lt(lhs, rhs);
            case ast_binary_oper_t::GT:
                return func->gt(lhs, rhs);
            case ast_binary_oper_t::ADD:
                return func->add(lhs, rhs);
            case ast_binary_oper_t::SUB:
                return func->sub(lhs, rhs);
            case ast_binary_oper_t::MUL:
                return func->mul(lhs, rhs);
            case ast_binary_oper_t::DIV:
                return func->div(lhs, rhs);
            case ast_binary_oper_t::MOD:
                return func->mod(lhs, rhs);
            case ast_binary_oper_t::BIT_AND:
                return func->b_and(lhs, rhs);
            case ast_binary_oper_t::BIT_OR:
                return func->b_or(lhs, rhs);
            case ast_binary_oper_t::BIT_XOR:
                return func->b_xor(lhs, rhs);
            case ast_binary_oper_t::SHIFT_L:
                return func->b_shl(lhs, rhs);
            case ast_binary_oper_t::SHIFT_R:
                return func->b_shr(lhs, rhs);
            default:
                return nullptr;
        }
    }

    omis_value_t* omis_module_coder_t::encode_expr_unary(ast_node_expr_unary_t* node) {
        if (node == nullptr)
            return nullptr;

        auto rhs = this->encode_expr(node->right);
        if (rhs == nullptr)
            return nullptr;

        auto func = this->scope->func;
        switch (node->op) {
            case ast_unary_oper_t::POS:
                return rhs;
            case ast_unary_oper_t::NEG:
                return func->neg(rhs);
            case ast_unary_oper_t::NOT:
                return func->l_not(rhs);
            case ast_unary_oper_t::FLIP:
                return func->b_flip(rhs);
            default:
                return nullptr;
        }
    }

    omis_value_t* omis_module_coder_t::encode_expr_int(ast_node_literal_int_t* node) {
        if (node == nullptr)
            return nullptr;

        u64_t vals = *((u64_t*) &(node->value));
        u32_t bits = vals > 0xFFFFFFFF ? 64 : 32;

        return this->value_integer(vals, bits);
    }

    omis_value_t* omis_module_coder_t::encode_expr_float(ast_node_literal_float_t* node) {
        if (node == nullptr)
            return nullptr;

        return this->value_float(node->value);
    }

    omis_value_t* omis_module_coder_t::encode_expr_bool(ast_node_literal_bool_t* node) {
        if (node == nullptr)
            return nullptr;
        return this->value_bool(node->value);
    }

    omis_value_t* omis_module_coder_t::encode_expr_string(ast_node_literal_string_t* node) {
        if (node == nullptr)
            return nullptr;

        // auto str = func->string_make(node->value.cstr());
        // return str;

        return nullptr;
    }

    omis_value_t* omis_module_coder_t::encode_expr_symbol_ref(ast_node_symbol_ref_t* node) {
        if (node == nullptr)
            return nullptr;

        auto* symbol = this->scope->get_value_symbol(node->name, true);
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
}
