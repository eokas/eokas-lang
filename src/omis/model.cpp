#include "./model.h"
#include "./bridge.h"

namespace eokas {
    omis_scope_t::omis_scope_t(omis_scope_t* parent, omis_func_t* func)
            : parent(parent), func(func), children(), types(), values() {}

    omis_scope_t::~omis_scope_t() {
        this->parent = nullptr;
        this->func = nullptr;
        _DeleteList(this->children);
    }

    omis_scope_t* omis_scope_t::add_child(omis_func_t* f) {
        auto* child = new omis_scope_t(this, f != nullptr ? f : this->func);
        this->children.push_back(child);
        return child;
    }

    bool omis_scope_t::add_type_symbol(const String& name, omis_type_t* type) {
        auto symbol = new omis_type_symbol_t{.name = name, .type = type};
        bool ret = this->types.add(name, symbol);
        return ret;
    }

    omis_type_symbol_t* omis_scope_t::get_type_symbol(const String& name, bool lookup) {
        if (lookup) {
            for (auto scope = this; scope != nullptr; scope = scope->parent) {
                auto* symbol = scope->types.get(name);
                if (symbol != nullptr)
                    return symbol;
            }
            return nullptr;
        } else {
            return this->types.get(name);
        }
    }

    omis_type_symbol_t* omis_scope_t::get_type_symbol(omis_lambda_predicate_t<omis_type_symbol_t> predicate, bool lookup) {
        if (lookup) {
            for (auto scope = this; scope != nullptr; scope = scope->parent) {
                auto* symbol = scope->types.get([&](auto name, auto symbol) -> auto {
                    return predicate(symbol);
                });
                if (symbol != nullptr)
                    return symbol;
            }
            return nullptr;
        } else {
            return this->types.get([&](auto name, auto symbol) -> auto {
                return predicate(symbol);
            });
        }
    }

    bool omis_scope_t::add_value_symbol(const String& name, omis_value_t* value) {
        auto symbol = new omis_value_symbol_t{.name =  name, .value = value, .scope = this};
        bool ret = this->values.add(name, symbol);
        return ret;
    }

    omis_value_symbol_t* omis_scope_t::get_value_symbol(const String& name, bool lookup) {
        if (lookup) {
            for (auto scope = this; scope != nullptr; scope = scope->parent) {
                auto symbol = scope->values.get(name);
                if (symbol != nullptr)
                    return symbol;
            }
            return nullptr;
        } else {
            return this->values.get(name);
        }
    }

    omis_value_symbol_t* omis_scope_t::get_value_symbol(omis_lambda_predicate_t<omis_value_symbol_t> predicate, bool lookup) {
        if (lookup) {
            for (auto scope = this; scope != nullptr; scope = scope->parent) {
                auto* symbol = scope->values.get([&](auto name, auto symbol) -> auto {
                    return predicate(symbol);
                });
                if (symbol != nullptr)
                    return symbol;
            }
            return nullptr;
        } else {
            return this->values.get([&](auto name, auto symbol) -> auto {
                return predicate(symbol);
            });
        }
    }
}

namespace eokas {
    omis_module_t::omis_module_t(omis_bridge_t* bridge, const String& name)
        : bridge(bridge)
        , name(name)
        , handle(nullptr)
        , root(new omis_scope_t(nullptr, nullptr))
        , scope(this->root)
        , usings()
        , types()
        , values() {
        this->handle = bridge->make_module(name.cstr());
    }

    omis_module_t::~omis_module_t() {
        _DeletePointer(root);
        _DeleteMap(types);
        _DeleteMap(values);

        this->bridge->drop_module(this->handle);
        this->handle = nullptr;
        this->bridge = nullptr;
    }

    bool omis_module_t::main() {
        return true;
    }

    const String& omis_module_t::get_name() const {
        return name;
    }

    omis_bridge_t* omis_module_t::get_bridge() {
        return bridge;
    }

    omis_handle_t omis_module_t::get_handle() {
        return handle;
    }

    String omis_module_t::dump() {
        return bridge->dump_module(handle);
    }

    bool omis_module_t::using_module(omis_module_t* other) {
        auto iter = std::find(usings.begin(), usings.end(), other);
        if (iter != usings.end())
            return false;
        usings.push_back(other);
        return true;
    }

    omis_scope_t* omis_module_t::get_scope() {
        return scope;
    }

    omis_scope_t* omis_module_t::push_scope(omis_func_t* func) {
        this->scope = this->scope->add_child(func);
    }

    omis_scope_t* omis_module_t::pop_scope() {
        if (this->scope == this->root)
            return this->root;
        this->scope = this->scope->parent;
    }

    omis_type_symbol_t* omis_module_t::get_type_symbol(const String& name, bool lookup) {
        return this->get_scope()->get_type_symbol(name, lookup);
    }

    bool omis_module_t::add_type_symbol(const String& name, omis_type_t* type) {
        return this->get_scope()->add_type_symbol(name, type);
    }

    omis_value_symbol_t* omis_module_t::get_value_symbol(const String& name, bool lookup) {
        return this->get_scope()->get_value_symbol(name, lookup);
    }

    bool omis_module_t::add_value_symbol(const String& name, omis_value_t* type) {
        return this->get_scope()->add_value_symbol(name, type);
    }

    omis_type_t* omis_module_t::type(omis_handle_t handle) {
        auto iter = this->types.find(handle);
        if (iter != this->types.end())
            return iter->second;

        auto type = new omis_type_t(this, handle);
        this->types.insert(std::make_pair(handle, type));

        return type;
    }

    omis_type_t* omis_module_t::type_void() {
        static omis_type_t* type = this->type(bridge->type_void());
        return type;
    }

    omis_type_t* omis_module_t::type_i8() {
        static omis_type_t* type = this->type(bridge->type_i8());
        return type;
    }

    omis_type_t* omis_module_t::type_i16() {
        static omis_type_t* type = this->type(bridge->type_i16());
        return type;
    }

    omis_type_t* omis_module_t::type_i32() {
        static omis_type_t* type = this->type(bridge->type_i32());
        return type;
    }

    omis_type_t* omis_module_t::type_i64() {
        static omis_type_t* type = this->type(bridge->type_i64());
        return type;
    }

    omis_type_t* omis_module_t::type_f32() {
        static omis_type_t* type = this->type(bridge->type_f32());
        return type;
    }

    omis_type_t* omis_module_t::type_f64() {
        static omis_type_t* type = this->type(bridge->type_f64());
        return type;
    }

    omis_type_t* omis_module_t::type_bool() {
        static omis_type_t* type = this->type(bridge->type_bool());
        return type;
    }

    omis_type_t* omis_module_t::type_bytes() {
        static omis_type_t* type = this->type(bridge->type_bytes());
        return type;
    }

    omis_type_t* omis_module_t::type_pointer(omis_type_t *type) {
        auto ret = bridge->type_pointer(type->get_handle());
        return this->type(ret);
    }

    omis_type_t* omis_module_t::type_func(omis_type_t* ret, const std::vector<omis_type_t*>& args, bool varg) {
        omis_handle_t ret_type = ret->get_handle();
        std::vector<omis_handle_t> args_type;
        for (auto& arg: args) {
            args_type.push_back(arg->get_handle());
        }
        omis_handle_t ft = bridge->type_func(ret_type, args_type, varg);
        return this->type(ft);
    }

    omis_value_t* omis_module_t::get_type_size(omis_type_t *type) {
        auto ret = bridge->get_type_size(type->get_handle());
        return this->value(ret);
    }

    bool omis_module_t::can_losslessly_bitcast(omis_type_t* a, omis_type_t* b) {
        return bridge->can_losslessly_cast(a->get_handle(), b->get_handle());
    }

    omis_value_t* omis_module_t::value(omis_type_t* type, omis_handle_t handle) {
        auto iter = this->values.find(handle);
        if (iter != this->values.end())
            return iter->second;

        auto val = new omis_value_t(this, type, handle);
        this->values.insert(std::make_pair(handle, val));

        return val;
    }

    omis_value_t* omis_module_t::value(omis_handle_t val) {
        auto type = this->type(bridge->get_value_type(val));
        return this->value(type, val);
    }

    omis_value_t* omis_module_t::value_integer(u64_t val, u32_t bits) {
        auto type = this->type_i64();
        if (bits == 32)
            type = this->type_i32();
        auto ret = bridge->value_integer(val, bits);
        return this->value(type, ret);
    }

    omis_value_t* omis_module_t::value_float(double val) {
        auto type = this->type_f64();
        auto ret = bridge->value_float(val);
        return this->value(type, ret);
    }

    omis_value_t* omis_module_t::value_bool(bool val) {
        auto type = this->type_bool();
        auto ret = bridge->value_bool(val);
        return this->value(type, ret);
    }

    omis_value_t* omis_module_t::value_string(const String& val) {
        return nullptr;
    }

    omis_func_t*
    omis_module_t::value_func(const String& name, omis_type_t* ret, const std::vector<omis_type_t*>& args, bool varg) {
        omis_handle_t ret_type = ret->get_handle();

        std::vector<omis_handle_t> args_types;
        for (auto& arg: args) {
            args_types.push_back(arg->get_handle());
        }

        auto type = this->type_func(ret, args, varg);
        auto func = bridge->value_func(this->handle, name, type->get_handle());

        auto iter = this->values.find(func);
        if(iter != this->values.end())
            return nullptr;
        auto value = new omis_func_t(this, type, func);
        this->values.insert(std::make_pair(func, value));

        return value;
    }

    bool omis_module_t::equals_type(omis_type_t* a, omis_type_t* b) {
        return a == b || a->get_handle() == b->get_handle();
    }

    bool omis_module_t::equals_value(omis_value_t *a, omis_value_t *b) {
        return a == b || a->get_handle() == b->get_handle();
    }
}

namespace eokas {
    omis_type_t::omis_type_t(omis_module_t* module, omis_handle_t handle)
            : module(module), handle(handle), default_value(nullptr) {

    }

    omis_type_t::~omis_type_t() {

    }

    omis_module_t* omis_type_t::get_module() {
        return module;
    }

    omis_handle_t omis_type_t::get_handle() {
        return handle;
    }

    omis_value_t* omis_type_t::get_default_value() {
        return default_value;
    }

    bool omis_type_t::is_type_func() {
        auto bridge = module->get_bridge();
        return bridge->is_type_func(this->handle);
    }

    bool omis_type_t::is_type_array() {
        auto bridge = module->get_bridge();
        return bridge->is_type_array(this->handle);
    }

    bool omis_type_t::is_type_struct() {
        auto bridge = module->get_bridge();
        return bridge->is_type_struct(this->handle);
    }

    omis_type_t* omis_type_t::get_pointer_type() {
        auto bridge = module->get_bridge();
        auto ret = bridge->type_pointer(this->handle);
        return module->type(ret);
    }

    omis_struct_t::omis_struct_t(omis_module_t* module, void* handle)
            : omis_type_t(module, handle) {

    }

    omis_struct_t::~omis_struct_t() {
        members.clear();
    }

    bool omis_struct_t::main() {
        return true;
    }

    bool omis_struct_t::extends(const String& base) {
        auto symbol = module->get_scope()->get_type_symbol(base, true);
        if (symbol == nullptr)
            return false;
        auto base_type = symbol->type;
        auto base_struct = dynamic_cast<omis_struct_t*>(base_type);
        if (base_struct == nullptr)
            return false;
        return this->extends(base_struct);
    }

    bool omis_struct_t::extends(omis_struct_t* base) {
        if (base == nullptr)
            return false;
        this->add_member("base", base);
        for (auto& m: base->members) {
            if (m.name == "base")
                continue;
            if (this->add_member(&m) == nullptr)
                return false;
        }
        return true;
    }

    omis_struct_t::member_t* omis_struct_t::add_member(const String& name, omis_type_t* type, omis_value_t* value) {
        if (this->get_member(name) != nullptr)
            return nullptr;
        if (type == nullptr && value == nullptr)
            return nullptr;

        member_t& m = this->members.emplace_back();
        m.name = name;
        m.type = type;
        m.value = value;

        if (type == nullptr) {
            m.type = value->get_type();
        }

        return &m;
    }

    omis_struct_t::member_t* omis_struct_t::add_member(const String& name, omis_value_t* value) {
        return this->add_member(name, nullptr, value);
    }

    omis_struct_t::member_t* omis_struct_t::add_member(omis_struct_t::member_t* other) {
        return this->add_member(other->name, other->type, other->value);
    }

    omis_struct_t::member_t* omis_struct_t::get_member(const String& name) {
        for (auto& m: this->members) {
            if (m.name == name)
                return &m;
        }
        return nullptr;
    }

    omis_struct_t::member_t* omis_struct_t::get_member(size_t index) {
        if (index >= this->members.size())
            return nullptr;
        member_t& m = this->members.at(index);
        return &m;
    }

    size_t omis_struct_t::get_member_index(const String& name) {
        for (size_t index = 0; index < this->members.size(); index++) {
            if (this->members.at(index).name == name)
                return index;
        }
        return -1;
    }
}

namespace eokas {
    omis_value_t::omis_value_t(omis_module_t *module, omis_type_t *type, void *handle)
            : module(module), type(type), handle(handle) {

    }

    omis_value_t::~omis_value_t() {

    }

    omis_module_t* omis_value_t::get_module() {
        return module;
    }

    omis_type_t* omis_value_t::get_type() {
        return type;
    }

    omis_handle_t omis_value_t::get_handle() {
        return handle;
    }

    void omis_value_t::set_name(const String &name) {
        auto bridge = module->get_bridge();
        bridge->set_value_name(this->handle, name);
    }

    omis_func_t::omis_func_t(omis_module_t *module, omis_type_t *type, void *handle)
            : omis_value_t(module, type, handle) {
        this->bridge = module->get_bridge();
    }

    omis_func_t::~omis_func_t() {
        this->bridge = nullptr;
    }

    omis_type_t* omis_func_t::get_ret_type() {
        auto type = this->type->get_handle();
        auto ret = bridge->get_func_ret_type(type);
        return module->type(ret);
    }

    uint32_t omis_func_t::get_arg_count() {
        auto type = this->type->get_handle();
        return bridge->get_func_arg_count(type);
    }

    omis_type_t* omis_func_t::get_arg_type(uint32_t index) {
        auto type = this->type->get_handle();
        auto ret = bridge->get_func_arg_type(type, index);
        return module->type(ret);
    }

    omis_value_t* omis_func_t::get_arg_value(uint32_t index) {
        auto ret = bridge->get_func_arg_value(this->handle, index);
        return module->value(ret);
    }

    omis_value_t* omis_func_t::create_block(const String &name) {
        auto ret = bridge->create_block(this->get_handle(), name);
        return module->value(module->type_void(), ret);
    }

    omis_value_t* omis_func_t::get_active_block() {
        auto ret = bridge->get_active_block();
        return module->value(module->type_void(), ret);
    }

    void omis_func_t::set_active_block(omis_value_t* block) {
        bridge->set_active_block(block->get_handle());
    }

    omis_value_t* omis_func_t::get_block_tail(omis_value_t* block) {
        auto ret = bridge->get_block_tail(block->get_handle());
        return module->value(ret);
    }

    bool omis_func_t::is_terminator_ins(omis_value_t *ins) {
        if(ins != nullptr) {
            return bridge->is_terminator_ins(ins->get_handle());
        }
        else {
            auto block = bridge->get_active_block();
            auto tail = bridge->get_block_tail(block);
            return bridge->is_terminator_ins(tail);
        }

    }

    omis_value_t* omis_func_t::load(omis_value_t *ptr) {
        auto ret = bridge->load(ptr->get_handle());
        return module->value(ret);
    }

    omis_value_t* omis_func_t::store(omis_value_t *ptr, omis_value_t *val) {
        auto ret = bridge->store(ptr->get_handle(), val->get_handle());
        return module->value(module->type_void(), ret);
    }

    omis_value_t* omis_func_t::neg(omis_value_t *a) {
        auto ret = bridge->neg(a->get_handle());
        return module->value(a->get_type(), ret);
    }

    omis_value_t* omis_func_t::add(omis_value_t *a, omis_value_t *b) {
        auto ret = bridge->add(a->get_handle(), b->get_handle());
        return module->value(ret);
    }

    omis_value_t* omis_func_t::sub(omis_value_t *a, omis_value_t *b) {
        auto ret = bridge->sub(a->get_handle(), b->get_handle());
        return module->value(ret);
    }

    omis_value_t* omis_func_t::mul(omis_value_t *a, omis_value_t *b) {
        auto ret = bridge->mul(a->get_handle(), b->get_handle());
        return module->value(ret);
    }

    omis_value_t* omis_func_t::div(omis_value_t *a, omis_value_t *b) {
        auto ret = bridge->div(a->get_handle(), b->get_handle());
        return module->value(ret);
    }

    omis_value_t* omis_func_t::mod(omis_value_t *a, omis_value_t *b) {
        auto ret = bridge->mod(a->get_handle(), b->get_handle());
        return module->value(ret);
    }

    omis_value_t* omis_func_t::eq(omis_value_t *a, omis_value_t *b) {
        auto ret = bridge->eq(a->get_handle(), b->get_handle());
        return module->value(module->type_bool(), ret);
    }

    omis_value_t* omis_func_t::ne(omis_value_t *a, omis_value_t *b) {
        auto ret = bridge->ne(a->get_handle(), b->get_handle());
        return module->value(module->type_bool(), ret);
    }

    omis_value_t* omis_func_t::gt(omis_value_t *a, omis_value_t *b) {
        auto ret = bridge->gt(a->get_handle(), b->get_handle());
        return module->value(module->type_bool(), ret);
    }

    omis_value_t* omis_func_t::ge(omis_value_t *a, omis_value_t *b) {
        auto ret = bridge->ge(a->get_handle(), b->get_handle());
        return module->value(module->type_bool(), ret);
    }

    omis_value_t* omis_func_t::lt(omis_value_t *a, omis_value_t *b) {
        auto ret = bridge->lt(a->get_handle(), b->get_handle());
        return module->value(module->type_bool(), ret);
    }

    omis_value_t* omis_func_t::le(omis_value_t *a, omis_value_t *b) {
        auto ret = bridge->le(a->get_handle(), b->get_handle());
        return module->value(module->type_bool(), ret);
    }

    omis_value_t* omis_func_t::l_not(omis_value_t *a) {
        auto ret = bridge->l_not(a->get_handle());
        return module->value(module->type_bool(), ret);
    }

    omis_value_t* omis_func_t::l_and(omis_value_t *a, omis_value_t *b) {
        auto ret = bridge->l_and(a->get_handle(), b->get_handle());
        return module->value(module->type_bool(), ret);
    }

    omis_value_t* omis_func_t::l_or(omis_value_t *a, omis_value_t *b) {
        auto ret = bridge->l_or(a->get_handle(), b->get_handle());
        return module->value(module->type_bool(), ret);
    }

    omis_value_t* omis_func_t::b_flip(omis_value_t *a) {
        auto ret = bridge->b_flip(a->get_handle());
        return module->value(a->get_type(), ret);
    }

    omis_value_t* omis_func_t::b_and(omis_value_t *a, omis_value_t *b) {
        auto ret = bridge->b_and(a->get_handle(), b->get_handle());
        return module->value(a->get_type(), ret);
    }

    omis_value_t* omis_func_t::b_or(omis_value_t *a, omis_value_t *b) {
        auto ret = bridge->b_or(a->get_handle(), b->get_handle());
        return module->value(a->get_type(), ret);
    }

    omis_value_t* omis_func_t::b_xor(omis_value_t *a, omis_value_t *b) {
        auto ret = bridge->b_xor(a->get_handle(), b->get_handle());
        return module->value(a->get_type(), ret);
    }

    omis_value_t* omis_func_t::b_shl(omis_value_t *a, omis_value_t *b) {
        auto ret = bridge->b_shl(a->get_handle(), b->get_handle());
        return module->value(a->get_type(), ret);
    }

    omis_value_t* omis_func_t::b_shr(omis_value_t *a, omis_value_t *b) {
        auto ret = bridge->b_shr(a->get_handle(), b->get_handle());
        return module->value(a->get_type(), ret);
    }

    omis_value_t* omis_func_t::jump(omis_value_t *pos) {
        auto ret = bridge->jump(pos->get_handle());
        return module->value(ret);
    }

    omis_value_t* omis_func_t::jump_cond(omis_value_t* cond, omis_value_t* branch_true, omis_value_t* branch_false) {
        auto ret = bridge->jump_cond(cond->get_handle(), branch_true->get_handle(), branch_false->get_handle());
        return module->value(ret);
    }

    omis_value_t* omis_func_t::phi(omis_type_t *type, const std::map<omis_value_t *, omis_value_t *>& incomings) {
        std::map<omis_handle_t, omis_handle_t> incomings_handles;
        for(auto& pair : incomings) {
            incomings_handles.insert(std::make_pair(pair.first->get_handle(), pair.second->get_handle()));
        }
        auto ret = bridge->phi(type->get_handle(), incomings_handles);
        return module->value(ret);
    }

    omis_value_t* omis_func_t::call(omis_func_t* func, const std::vector<omis_value_t*>& args) {
        std::vector<omis_handle_t> args_values;
        for(auto& arg : args) {
            args_values.push_back(arg->get_handle());
        }
        auto ret = bridge->call(func->get_handle(), args_values);
        return module->value(ret);
    }

    omis_value_t* omis_func_t::call(const String &func_name, const std::vector<omis_value_t *> &args) {
        auto symbol = module->get_value_symbol(func_name);
        if(symbol == nullptr)
            return nullptr;
        auto func = dynamic_cast<omis_func_t*>(symbol->value);
        if(func == nullptr)
            return nullptr;
        return this->call(func, args);
    }

    omis_value_t* omis_func_t::ret(omis_value_t* value) {
        auto ret = bridge->ret(value != nullptr ? value->get_handle() : nullptr);
        return module->value(ret);
    }

    omis_value_t* omis_func_t::bitcast(omis_value_t *value, omis_type_t *type) {
        auto ret = bridge->bitcast(value->get_handle(), type->get_handle());
        return module->value(ret);
    }

    omis_value_t* omis_func_t::create_local_symbol(const String &name, omis_type_t *type, omis_value_t *value) {
        auto ptr = bridge->alloc(type->get_handle(), name);
        auto ret = bridge->store(ptr, value->get_handle());
        return module->value(ptr);
    }

    void omis_func_t::ensure_tail_ret() {
        auto block = bridge->get_active_block();
        auto lastOp = bridge->get_block_tail(block);

        if(!bridge->is_terminator_ins(lastOp)) {
            auto ret_type = bridge->get_func_ret_type(this->type->get_handle());
            if (ret_type == module->type_void()->get_handle())
                bridge->ret();
            else
                bridge->ret(bridge->get_default_value(ret_type));
        }
    }

    omis_value_t* omis_func_t::get_ptr_val(omis_value_t *ptr) {
        auto ret = bridge->get_ptr_val(ptr->get_handle());
        return module->value(ret);
    }

    omis_value_t* omis_func_t::get_ptr_ref(omis_value_t *ptr) {
        auto ret = bridge->get_ptr_ref(ptr->get_handle());
        return module->value(ret);
    }

    omis_value_t* omis_func_t::make(omis_type_t* type) {
        auto len = module->get_type_size(type);
        auto ptr = this->call("malloc", {len});

        this->stmt_if(
            [&]()->auto {
                return this->eq(ptr, module->value_integer(0, 64));
            },
            [&]()->auto{
                return true;
            },
            nullptr
        );

        auto val = this->bitcast(ptr, type);
        return val;
    }

    omis_value_t* omis_func_t::make(omis_type_t* type, omis_value_t* count) {
        auto stride = module->get_type_size(type);
        auto len = this->mul(stride, count);
        auto ptr = this->call("malloc", {len});
        auto val = this->bitcast(ptr, type);
        return val;
    }

    omis_value_t* omis_func_t::drop(omis_value_t* ptr) {
        return this->call("free", {ptr});
    }

    omis_value_t* omis_func_t::expr(const omis_lambda_expr_t& lambda) {
        return lambda();
    }

    bool omis_func_t::stmt(const omis_lambda_stmt_t& lambda) {
        if(lambda.has_value()) {
            if(!lambda.value()()) {
                return false;
            }
        }
        return true;
    }

    bool omis_func_t::stmt_if(const omis_lambda_expr_t& lambda_cond,
                              const omis_lambda_stmt_t& lambda_true,
                              const omis_lambda_stmt_t& lambda_false) {

        auto if_true = this->create_block("if.true");
        auto if_false = this->create_block("if.false");
        auto if_end = this->create_block("if.end");

        auto cond = this->expr(lambda_cond);
        if (cond == nullptr)
            return false;
        cond = this->get_ptr_val(cond);
        if (!module->equals_type(cond->get_type(), module->type_bool())) {
            printf("ERROR: The label 'if.cond' need a bool value.\n");
            return false;
        }
        this->jump_cond(cond, if_true, if_false);

        // if-true
        this->set_active_block(if_true);
        {
            if (!this->stmt(lambda_true))
                return false;
            auto active_block = this->get_active_block();
            if (!module->equals_value(active_block, if_true) && !this->is_terminator_ins(active_block)) {
                this->jump(if_end);
            }
            if (!this->is_terminator_ins(this->get_block_tail(if_true))) {
                this->jump(if_end);
            }
        }

        // if-false
        this->set_active_block(if_false);
        {
            if (!this->stmt(lambda_false))
                return false;
            auto active_block = this->get_active_block();
            if (!module->equals_value(active_block, if_false) && !this->is_terminator_ins(active_block)) {
                this->jump(if_end);
            }
            if (!this->is_terminator_ins(this->get_block_tail(if_false))) {
                this->jump(if_end);
            }
        }

        this->set_active_block(if_end);

        return true;
    }

    bool omis_func_t::stmt_loop(const omis_lambda_stmt_t& lambda_init,
                                const omis_lambda_expr_t& lambda_cond,
                                const omis_lambda_stmt_t& lambda_step,
                                const omis_lambda_stmt_t& lambda_body) {
        module->push_scope();

        auto loop_cond = this->create_block("loop.cond");
        auto loop_step = this->create_block("loop.step");
        auto loop_body = this->create_block("loop.body");
        auto loop_end = this->create_block("loop.end");

        auto old_continue_point = this->continue_point;
        auto old_break_point = this->break_point;
        this->continue_point = loop_step;
        this->break_point = loop_end;

        // init
        {
            if (!this->stmt(lambda_init))
                return false;
            this->jump(loop_cond);
        }

        // cond
        this->set_active_block(loop_cond);
        {
            auto cond = this->expr(lambda_cond);
            if (cond == nullptr)
                return false;
            cond = this->get_ptr_val(cond);
            if (!module->equals_type(cond->get_type(), module->type_bool())) {
                printf("ERROR: The label 'loop.cond' need a bool value.\n");
                return false;
            }
            this->jump_cond(cond, loop_body, loop_end);
        }

        // body
        this->set_active_block(loop_body);
        {
            if (!this->stmt(lambda_body))
                return false;
            auto active_block = this->get_active_block();
            if (!module->equals_value(active_block, loop_body) &&
                !this->is_terminator_ins(active_block)) {
                this->jump(loop_step);
            }
            if (!this->is_terminator_ins(this->get_block_tail(loop_body))) {
                this->jump(loop_step);
            }
        }

        // step
        this->set_active_block(loop_step);
        {
            if (!this->stmt(lambda_step))
                return false;
            this->jump(loop_cond);
        }

        this->set_active_block(loop_end);

        this->continue_point = old_continue_point;
        this->break_point = old_break_point;

        module->pop_scope();

        return true;
    }

    bool omis_func_t::stmt_break() {
        if (this->break_point == nullptr)
            return false;
        this->jump(this->break_point);
        return true;
    }

    bool omis_func_t::stmt_continue() {
        if (this->continue_point == nullptr)
            return false;
        this->jump(this->continue_point);
        return true;
    }
}
