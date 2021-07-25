
#include "coder.h"
#include "ast.h"

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

struct llvm_type_t
{
    enum class category_t
    {
        Basic,
        Schema,
        Struct,
    };

    category_t category;
    String name;
    llvm::Type* value;

    llvm_type_t(category_t category, const String& name, llvm::Type* value)
        :category(category)
        ,name(name)
        ,value(value)
    {}
    virtual ~llvm_type_t()
    {}
};

struct llvm_type_basic_t :public llvm_type_t
{
    llvm_type_basic_t(const String& name, llvm::Type* value)
        :llvm_type_t(category_t::Basic, name, value)
    {}
};

struct llvm_type_schema_t :public llvm_type_t
{
    std::map<String, llvm_type_t*> members;

    llvm_type_schema_t(llvm::LLVMContext& context, const String& name)
        :llvm_type_t(category_t::Schema, name, llvm::PointerType::get(llvm::StructType::get(context), 0))
        ,members()
    {}

    bool match(const std::map<String, llvm_type_t*> body)
    {
        for(auto node : this->members)
        {
            const String& name = node.first;
            const llvm_type_t* type = node.second;

            auto iter = body.find(name);
            if(iter == body.end())
                return false;
            if(iter->second->name != type->name)
                return false;
        }
        return true;
    }
};

struct llvm_type_struct_t :public llvm_type_t
{
    std::map<String, llvm_type_t*> members;

    llvm_type_struct_t(llvm::LLVMContext& context, const String& name)
        :llvm_type_t(category_t::Schema, name, llvm::PointerType::get(llvm::StructType::get(context), 0))
        ,members()
    {}
};

static bool llvm_type_can_be_cast(llvm_type_t* type1, llvm_type_t* type2)
{
    if(type1 == nullptr || type2 == nullptr)
        return false;
    if ((type1->category != type2->category) && 
        (type1->category == llvm_type_t::category_t::Basic || type2->category == llvm_type_t::category_t::Basic))
    {
        return false;
    }

    
    return true;
}

struct llvm_scope_t
{
    llvm_scope_t* parent;
    std::vector<llvm_scope_t*> children;

    std::map<String, llvm_type_t*> types;
    std::map<String, llvm::Value*> symbols;

    llvm_scope_t(llvm_scope_t* parent)
        : parent(parent)
        , children()
        , types()
        , symbols()
    {}

    virtual ~llvm_scope_t()
    {
        this->parent = nullptr;
        _DeleteList(this->children);
        _DeleteMap(this->types);
        this->symbols.clear();
    }

    llvm_scope_t* addChild()
    {
        llvm_scope_t* child = new llvm_scope_t(this);
        this->children.push_back(child);
        return child;
    }

    llvm_type_t* getType(const String& name, bool lookUp)
    {
        if (lookUp)
        {
            for (auto scope = this; scope != nullptr; scope = scope->parent)
            {
                auto iter = scope->types.find(name);
                if (iter != scope->types.end())
                    return iter->second;
            }
            return nullptr;
        }
        else
        {
            auto iter = this->types.find(name);
            if (iter != this->types.end())
                return iter->second;
            return nullptr;
        }
    }
    
    llvm::Value* getSymbol(const String& name, bool lookUp)
    {
        if (lookUp)
        {
            for (auto scope = this; scope != nullptr; scope = scope->parent)
            {
                auto iter = scope->symbols.find(name);
                if (iter != scope->symbols.end())
                    return iter->second;
            }
            return nullptr;
        }
        else
        {
            auto iter = this->symbols.find(name);
            if (iter != this->symbols.end())
                return iter->second;
            return nullptr;
        }
    }
};

struct llvm_coder_t
{
    Stream& base;

    std::unique_ptr<llvm::LLVMContext> llvm_context;
    std::unique_ptr<llvm::Module> llvm_module;
    std::unique_ptr<llvm::IRBuilder<>> llvm_builder;
    llvm_scope_t* root;
    llvm_scope_t* scope;
    llvm::Function* func;
    llvm::BasicBlock* continuePoint;
    llvm::BasicBlock* breakPoint;

    llvm_coder_t(Stream& stream)
        : base(stream)
    {
        this->llvm_context = std::make_unique<llvm::LLVMContext>();
        this->llvm_module = std::make_unique<llvm::Module>("eokas", *llvm_context);
        this->llvm_builder = std::make_unique<llvm::IRBuilder<>>(*llvm_context);
        this->root = new llvm_scope_t(nullptr);
        this->scope = this->root;
        this->func = nullptr;
        this->continuePoint = nullptr;
        this->breakPoint = nullptr;
    }

    virtual ~llvm_coder_t()
    {
        _DeletePointer(this->scope);
    }

    void pushScope()
    {
        this->scope = this->scope->addChild();
    }

    void popScope()
    {
        this->scope = this->scope->parent;
    }

    bool encode(struct ast_module_t* m)
    {
        if (!this->encode_module(m))
            return false;

        // DUMP CODE
        llvm_module->print(llvm::errs(), nullptr);

        return true;
    }

    bool encode_module(struct ast_module_t* node)
    {
        if (node == nullptr)
            return false;

        this->pushScope();

        this->scope->types["i8"] = new llvm_type_basic_t("i8", llvm::Type::getInt8Ty(*llvm_context));
        this->scope->types["i16"] = new llvm_type_basic_t("i16", llvm::Type::getInt16Ty(*llvm_context));
        this->scope->types["i32"] = new llvm_type_basic_t("i32", llvm::Type::getInt32Ty(*llvm_context));
        this->scope->types["i64"] = new llvm_type_basic_t("i64", llvm::Type::getInt64Ty(*llvm_context));
        this->scope->types["u8"] = new llvm_type_basic_t("u8", llvm::Type::getInt8Ty(*llvm_context));
        this->scope->types["u16"] = new llvm_type_basic_t("u16", llvm::Type::getInt16Ty(*llvm_context));
        this->scope->types["u32"] = new llvm_type_basic_t("u32", llvm::Type::getInt32Ty(*llvm_context));
        this->scope->types["u64"] = new llvm_type_basic_t("u64", llvm::Type::getInt64Ty(*llvm_context));
        this->scope->types["f32"] = new llvm_type_basic_t("f32", llvm::Type::getFloatTy(*llvm_context));
        this->scope->types["f64"] = new llvm_type_basic_t("f64", llvm::Type::getDoubleTy(*llvm_context));
        this->scope->types["bool"] = new llvm_type_basic_t("bool", llvm::Type::getInt1Ty(*llvm_context));

        llvm::FunctionType* funcType = llvm::FunctionType::get(llvm::Type::getVoidTy(*llvm_context), false);
        this->func = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, "@_main", *llvm_module);

        llvm::BasicBlock* entry = llvm::BasicBlock::Create(*llvm_context, "entry", this->func);
        llvm_builder->SetInsertPoint(entry);

        for (auto& stmt : node->stmts)
        {
            if (!this->encode_stmt(stmt))
                return false;
        }

        llvm_builder->CreateRetVoid();

        this->popScope();

        return true;
    }

    llvm_type_t* encode_type(struct ast_type_t* node)
    {
        if (node == nullptr)
            return nullptr;

        switch (node->category)
        {
        case ast_node_category_t::type_ref:
            return this->encode_type_ref(dynamic_cast<ast_type_ref_t*>(node));
        default:
            return nullptr;
        }

        return nullptr;
    }

    llvm_type_t* encode_type_ref(struct ast_type_ref_t* node)
    {
        if (node == nullptr)
        {
            printf("type node is null. \n");
            return nullptr;
        }

        const String& name = node->name;
        llvm_type_t* type = this->scope->getType(name, true);
        return type;
    }

    llvm::Value* encode_expr(struct ast_expr_t* node)
    {
        if (node == nullptr)
            return nullptr;

        switch (node->category)
        {
        case ast_node_category_t::expr_trinary:
            return this->encode_expr_trinary(dynamic_cast<ast_expr_trinary_t*>(node));
        case ast_node_category_t::expr_binary:
            return this->encode_expr_binary(dynamic_cast<ast_expr_binary_t*>(node));
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
        default:
            return nullptr;
        }
        return nullptr;
    }

    llvm::Value* encode_expr_trinary(struct ast_expr_trinary_t* node)
    {
        if (node == nullptr)
            return nullptr;

        llvm::BasicBlock* trinary_begin = llvm::BasicBlock::Create(*llvm_context, "trinary.begin", this->func);
        llvm::BasicBlock* trinary_true = llvm::BasicBlock::Create(*llvm_context, "trinary.true", this->func);
        llvm::BasicBlock* trinary_false = llvm::BasicBlock::Create(*llvm_context, "trinary.false", this->func);
        llvm::BasicBlock* trinary_end = llvm::BasicBlock::Create(*llvm_context, "trinary.end", this->func);

        llvm_builder->SetInsertPoint(trinary_begin);

        // TODO: define a temp var

        llvm::Value* cond = this->encode_expr(node->cond);
        if (cond == nullptr)
            return nullptr;
        llvm_builder->CreateCondBr(cond, trinary_true, trinary_false);

        llvm_builder->SetInsertPoint(trinary_true);
        llvm::Value* trueV = this->encode_expr(node->branch_true);
        if (trueV == nullptr)
            return nullptr;
        // TODO: set temp var
        llvm_builder->CreateBr(trinary_end);

        llvm_builder->SetInsertPoint(trinary_false);
        llvm::Value* falseV = this->encode_expr(node->branch_false);
        if (falseV == nullptr)
            return nullptr;
        // TODO: set temp var
        llvm_builder->CreateBr(trinary_end);

        llvm_builder->SetInsertPoint(trinary_end);

        // TODO: return temp var

        return nullptr;
    }

    llvm::Value* encode_expr_binary(struct ast_expr_binary_t* node)
    {
        if (node == nullptr)
            return nullptr;

        auto lhs = this->encode_expr(node->left);
        auto rhs = this->encode_expr(node->right);
        if (lhs == nullptr || rhs == nullptr)
            return nullptr;

        switch (node->op)
        {
        case ast_binary_oper_t::Or:
            return llvm_builder->CreateOr(lhs, rhs);
        case ast_binary_oper_t::And:
            return llvm_builder->CreateAnd(lhs, rhs);
        case ast_binary_oper_t::Equal:
            return llvm_builder->CreateICmpEQ(lhs, rhs);
        case ast_binary_oper_t::NEqual:
            return llvm_builder->CreateICmpNE(lhs, rhs);
        case ast_binary_oper_t::LEqual:
            return llvm_builder->CreateICmpSLE(lhs, rhs);
        case ast_binary_oper_t::GEqual:
            return llvm_builder->CreateICmpSGE(lhs, rhs);
        case ast_binary_oper_t::Less:
            return llvm_builder->CreateICmpSLT(lhs, rhs);
        case ast_binary_oper_t::Greater:
            return llvm_builder->CreateICmpSGT(lhs, rhs);
        case ast_binary_oper_t::Add:
            return llvm_builder->CreateAdd(lhs, rhs);
        case ast_binary_oper_t::Sub:
            return llvm_builder->CreateSub(lhs, rhs);
        case ast_binary_oper_t::Mul:
            return llvm_builder->CreateMul(lhs, rhs);
        case ast_binary_oper_t::Div:
            return llvm_builder->CreateSDiv(lhs, rhs);
        case ast_binary_oper_t::Mod:
            return llvm_builder->CreateSRem(lhs, rhs);
        case ast_binary_oper_t::BitAnd:
            return nullptr;
        case ast_binary_oper_t::BitOr:
            return nullptr;
        case ast_binary_oper_t::BitXor:
            return llvm_builder->CreateXor(lhs, rhs);
        case ast_binary_oper_t::ShiftL:
            return llvm_builder->CreateShl(lhs, rhs);
        case ast_binary_oper_t::ShiftR:
            return llvm_builder->CreateLShr(lhs, rhs);
        default:
            return nullptr;
        }
    }

    llvm::Value* encode_expr_unary(struct ast_expr_unary_t* node)
    {
        if (node == nullptr)
            return nullptr;

        auto rhs = this->encode_expr(node->right);
        if (rhs == nullptr)
            return nullptr;


        switch (node->op)
        {
        case ast_unary_oper_t::Pos:
            return rhs;
        case ast_unary_oper_t::Neg:
            return llvm_builder->CreateNeg(rhs);
        case ast_unary_oper_t::Not:
            return llvm_builder->CreateNot(rhs);
        case ast_unary_oper_t::Flip:
            return nullptr;
        default:
            return nullptr;
        }
    }

    llvm::Value* encode_expr_int(struct ast_expr_int_t* node)
    {
        if (node == nullptr)
            return nullptr;

        u64_t vals = *((u64_t*)&(node->value));
        u32_t bits = 8;
        if (vals > 0xFF) bits = 16;
        if (vals > 0xFFFF) bits = 32;
        if (vals > 0xFFFFFFFF) bits = 64;

        return llvm::ConstantInt::get(*llvm_context, llvm::APInt(bits, node->value));
    }

    llvm::Value* encode_expr_float(struct ast_expr_float_t* node)
    {
        if (node == nullptr)
            return nullptr;

        return llvm::ConstantFP::get(*llvm_context, llvm::APFloat(node->value));
    }

    llvm::Value* encode_expr_bool(struct ast_expr_bool_t* node)
    {
        if (node == nullptr)
            return nullptr;
        return llvm::ConstantInt::getBool(*llvm_context, node->value);
    }

    llvm::Value* encode_expr_string(struct ast_expr_string_t* node)
    {
        if (node == nullptr)
            return nullptr;
        return llvm::ConstantDataArray::getString(*llvm_context, node->value.cstr());
    }

    llvm::Value* encode_expr_symbol_ref(struct ast_expr_symbol_ref_t* node)
    {
        if (node == nullptr)
            return nullptr;

        auto symbol = this->scope->getSymbol(node->name, true);
        if (symbol == nullptr)
            return nullptr;

        return llvm_builder->CreateLoad(symbol);
    }

    llvm::Value* encode_expr_func_def(struct ast_expr_func_def_t* node)
    {
        if (node == nullptr)
            return nullptr;

        llvm_type_t* retType = this->encode_type(node->type);
        if(retType == nullptr)
            return nullptr;

        std::vector<llvm::Type*> argTypes;
        for (auto arg : node->args)
        {
            llvm_type_t* argType = this->encode_type(arg.type);
            if (argType == nullptr)
                return nullptr;
            argTypes.push_back(argType->value);
        }

        llvm::FunctionType* procType = llvm::FunctionType::get(retType->value, argTypes, false);
        llvm::Function* func = llvm::Function::Create(procType, llvm::Function::ExternalLinkage, "", *llvm_module);
        u32_t index = 0;
        for (auto& arg : func->args())
        {
            const char* name = node->args[index++].name.cstr();
            arg.setName(name);
        }

        auto oldFunc = this->func;
        auto oldIB = llvm_builder->GetInsertBlock();
        this->func = func;
        this->pushScope();
        {
            llvm::BasicBlock* entry = llvm::BasicBlock::Create(*llvm_context, "entry", func);
            llvm_builder->SetInsertPoint(entry);

            for (auto& arg : func->args())
            {
                const char* name = arg.getName().data();
                llvm::Value* ptr = llvm_builder->CreateAlloca(arg.getType());
                llvm_builder->CreateStore(&arg, ptr);
                auto pair = std::make_pair(String(name), ptr);
                this->scope->symbols.insert(pair);
            }

            for (auto& stmt : node->body)
            {
                if (!this->encode_stmt(stmt))
                    return nullptr;
            }
        }

        this->popScope();
        this->func = oldFunc;
        llvm_builder->SetInsertPoint(oldIB);

        return func;
    }

    llvm::Value* encode_expr_func_ref(struct ast_expr_func_ref_t* node)
    {
        if (node == nullptr)
            return nullptr;

        llvm::Value* funcValue = this->encode_expr(node->func);
        llvm::Function* func = llvm::cast<llvm::Function>(funcValue);
        if (func == nullptr)
            return nullptr;
        if (func->arg_size() != node->args.size())
            return nullptr;

        std::vector<llvm::Value*> params;
        for (auto& arg : node->args)
        {
            llvm::Value* param = this->encode_expr(arg);
            if (param == nullptr)
                return nullptr;
            params.push_back(param);
        }

        return llvm_builder->CreateCall(func, params);
    }

    llvm::Value* encode_expr_array_def(struct ast_expr_array_def_t* node)
    {
        if (node == nullptr)
            return nullptr;

        return nullptr;
    }

    llvm::Value* encode_expr_index_ref(struct ast_expr_index_ref_t* node)
    {
        if (node == nullptr)
            return nullptr;

        return nullptr;
    }

    bool encode_stmt(struct ast_stmt_t* node)
    {
        if (node == nullptr)
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
        printf("1 \n");
        if (node == nullptr)
            return false;

        printf("2 \n");
        const String& name = node->name;

        printf("3 \n");
        std::map<String, llvm_type_t*> members;
        for (auto node : node->members)
        {
            auto mem = node.second;
            printf("4 \n");
            auto memType = this->encode_type(mem->type);
            printf("5 \n");
            if (memType == nullptr)
                return false;
            members[mem->name] = memType;
        }
        printf("6 \n");
        auto type = new llvm_type_schema_t(*llvm_context, name);
        type->members = members;
        printf("7 schema.members: %d \n", type->members.size());

        this->scope->types.insert(std::make_pair(name, type));

        return true;
    }

    bool encode_stmt_struct_def(struct ast_stmt_struct_def_t* node)
    {
        printf("1 \n");
        if (node == nullptr)
            return false;

        printf("2 \n");
        const String& name = node->name;

        printf("3 \n");
        std::map<String, llvm_type_t*> members;
        for (auto node : node->members)
        {
            auto mem = node.second;
            printf("4 \n");
            auto memType = this->encode_type(mem->type);
            printf("5 \n");
            if (memType == nullptr)
                return false;
            members[mem->name] = memType;
        }
        printf("6 \n");

        if (node->schema != nullptr)
        {
            const String& schemaName = node->schema->name;
            auto schemaType = dynamic_cast<llvm_type_schema_t*>(this->scope->getType(schemaName, true));
            if(schemaType == nullptr)
                return false;
            
            if(!schemaType->match(members))
                return false;
        }

        auto type = new llvm_type_struct_t(*llvm_context, name);
        type->members = members;

        this->scope->types.insert(std::make_pair(name, type));

        return true;
    }

    bool encode_stmt_proc_def(struct ast_stmt_proc_def_t* node)
    {
        if (node == nullptr)
            return false;

        if (this->scope->getType(node->name, false) != nullptr)
            return false;

        llvm::Type* retType = this->encode_type(node->type);

        std::vector<llvm::Type*> argTypes;
        for (auto arg : node->args)
        {
            llvm::Type* argType = this->encode_type(arg.second);
            if (argType == nullptr)
                return false;
            argTypes.push_back(argType);
        }

        llvm::FunctionType* procType = llvm::FunctionType::get(retType, argTypes, false);

        this->scope->types[node->name] = procType;

        return true;
    }

    bool encode_stmt_symbol_def(struct ast_stmt_symbol_def_t* node)
    {
        if (node == nullptr)
            return false;

        if (this->scope->getSymbol(node->name, false) != nullptr)
            return false;

        auto type = this->encode_type(node->type);
        auto value = this->encode_expr(node->value);
        if (value == nullptr)
            return false;

        auto vtype = value->getType();
        if (type != nullptr)
        {
            if (type->value->getTypeID() != vtype->getTypeID() && !vtype->canLosslesslyBitCastTo(type->value))
            {
                printf("type '%d' can not cast to type '%d'. \n", vtype->getTypeID(), type->value->getTypeID());
                return false;
            }
        }
        else {
            type = new llvm_type_basic_t();
        }

        // TODO: 校验类型合法性, 值类型是否遵循标记类型
        // 不同的类型，需要调用不同的store命令

        auto symbol = llvm_builder->CreateAlloca(type->value);
        llvm_builder->CreateStore(value, symbol);
        scope->symbols.insert(std::make_pair(node->name, symbol));

        return true;
    }

    bool encode_stmt_break(struct ast_stmt_break_t* node)
    {
        if (node == nullptr)
            return false;

        if (this->breakPoint == nullptr)
            return false;

        llvm_builder->CreateBr(this->breakPoint);

        return true;
    }

    bool encode_stmt_continue(struct ast_stmt_continue_t* node)
    {
        if (node == nullptr)
            return false;

        if (this->continuePoint == nullptr)
            return false;

        llvm_builder->CreateBr(this->continuePoint);

        return true;
    }

    bool encode_stmt_return(struct ast_stmt_return_t* node)
    {
        if (node == nullptr)
            return false;

        if (node->value != nullptr)
        {
            llvm::Value* value = this->encode_expr(node->value);
            llvm_builder->CreateRet(value);
        }
        else
        {
            llvm_builder->CreateRetVoid();
        }

        return true;
    }

    bool encode_stmt_if(struct ast_stmt_if_t* node)
    {
        if (node == nullptr)
            return false;

        llvm::BasicBlock* if_begin = llvm::BasicBlock::Create(*llvm_context, "if.begin", this->func);
        llvm::BasicBlock* if_true = llvm::BasicBlock::Create(*llvm_context, "if.true", this->func);
        llvm::BasicBlock* if_false = llvm::BasicBlock::Create(*llvm_context, "if.false", this->func);
        llvm::BasicBlock* if_end = llvm::BasicBlock::Create(*llvm_context, "if.end", this->func);

        llvm_builder->SetInsertPoint(if_begin);
        llvm::Value* cond = this->encode_expr(node->cond);
        if (cond == nullptr)
            return false;
        llvm_builder->CreateCondBr(cond, if_true, if_false);

        llvm_builder->SetInsertPoint(if_true);
        if (!this->encode_stmt(node->branch_true))
            return false;
        llvm_builder->CreateBr(if_end);

        if (node->branch_false != nullptr)
        {
            llvm_builder->SetInsertPoint(if_false);
            if (!this->encode_stmt(node->branch_false))
                return false;
            llvm_builder->CreateBr(if_end);
        }

        llvm_builder->SetInsertPoint(if_end);

        return true;
    }

    bool encode_stmt_while(struct ast_stmt_while_t* node)
    {
        if (node == nullptr)
            return false;

        this->pushScope();

        llvm::BasicBlock* while_begin = llvm::BasicBlock::Create(*llvm_context, "while.begin", this->func);
        llvm::BasicBlock* while_body = llvm::BasicBlock::Create(*llvm_context, "while.body", this->func);
        llvm::BasicBlock* while_end = llvm::BasicBlock::Create(*llvm_context, "while.end", this->func);

        auto oldContinuePoint = this->continuePoint;
        auto oldBreakPoint = this->breakPoint;

        this->continuePoint = while_begin;
        this->breakPoint = while_end;

        llvm_builder->SetInsertPoint(while_begin);
        llvm::Value* cond = this->encode_expr(node->cond);
        if (cond == nullptr)
            return false;
        llvm_builder->CreateCondBr(cond, while_body, while_end);

        llvm_builder->SetInsertPoint(while_body);
        if (!this->encode_stmt(node->body))
            return false;
        llvm_builder->CreateBr(while_begin);

        llvm_builder->SetInsertPoint(while_end);

        this->continuePoint = oldContinuePoint;
        this->breakPoint = oldBreakPoint;

        this->popScope();

        return true;
    }

    bool encode_stmt_for(struct ast_stmt_for_t* node)
    {
        if (node == nullptr)
            return false;

        this->pushScope();

        llvm::BasicBlock* for_init = llvm::BasicBlock::Create(*llvm_context, "for.init", this->func);
        llvm::BasicBlock* for_test = llvm::BasicBlock::Create(*llvm_context, "for.test", this->func);
        llvm::BasicBlock* for_step = llvm::BasicBlock::Create(*llvm_context, "for.step", this->func);
        llvm::BasicBlock* for_body = llvm::BasicBlock::Create(*llvm_context, "for.body", this->func);
        llvm::BasicBlock* for_end = llvm::BasicBlock::Create(*llvm_context, "for.end", this->func);

        auto oldContinuePoint = this->continuePoint;
        auto oldBreakPoint = this->breakPoint;
        this->continuePoint = for_step;
        this->breakPoint = for_end;

        llvm_builder->SetInsertPoint(for_init);
        if (!this->encode_stmt(node->init))
            return false;
        llvm_builder->CreateBr(for_test);

        llvm_builder->SetInsertPoint(for_test);
        llvm::Value* cond = this->encode_expr(node->cond);
        if (cond == nullptr)
            return false;
        llvm_builder->CreateCondBr(cond, for_body, for_end);

        llvm_builder->SetInsertPoint(for_body);
        if (!this->encode_stmt(node->body))
            return false;
        llvm_builder->CreateBr(for_step);

        llvm_builder->SetInsertPoint(for_step);
        if (!this->encode_stmt(node->step))
            return false;
        llvm_builder->CreateBr(for_test);

        llvm_builder->SetInsertPoint(for_end);

        this->continuePoint = oldContinuePoint;
        this->breakPoint = oldBreakPoint;

        this->popScope();

        return true;
    }

    bool encode_stmt_block(struct ast_stmt_block_t* node)
    {
        if (node == nullptr)
            return false;

        this->pushScope();

        for (auto& stmt : node->stmts)
        {
            if (!this->encode_stmt(stmt))
                return false;
        }

        this->popScope();

        return true;
    }

    bool encode_stmt_assign(struct ast_stmt_assign_t* node)
    {
        if (node == nullptr)
            return false;

        llvm::Value* left = this->encode_expr(node->left);
        llvm::Value* right = this->encode_expr(node->right);

        llvm_builder->CreateStore(right, left);

        return true;
    }

    bool encode_stmt_call(struct ast_stmt_call_t* node)
    {
        if (node == nullptr)
            return false;

        if (!this->encode_expr_func_ref(node->expr))
            return false;

        return true;
    }
};


coder_t::coder_t(Stream& stream)
    : impl(new llvm_coder_t(stream))
{}

coder_t::~coder_t()
{
    _DeletePointer(impl);
}

bool coder_t::encode(struct ast_module_t* m)
{
    return impl->encode(m);
}

_EndNamespace(eokas)
