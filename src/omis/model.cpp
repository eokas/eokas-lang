#include "./model.h"
#include "./scope.h"
#include "./bridge.h"

namespace eokas {
    omis_module_t::omis_module_t(const String& name, omis_bridge_t* bridge)
            : name(name), bridge(bridge), root(new omis_scope_t(nullptr, nullptr)), scope(this->root), usings(),
              types(), values() {

    }

    omis_module_t::~omis_module_t() {
        _DeletePointer(root);
        _DeleteMap(types);
        _DeleteMap(values);
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

    omis_type_t* omis_module_t::type_func(omis_type_t* ret, const std::vector<omis_type_t*>& args, bool varg) {
        omis_handle_t ret_type = ret->get_handle();
        std::vector<omis_handle_t> args_type;
        for (auto& arg: args) {
            args_type.push_back(arg->get_handle());
        }
        omis_handle_t handle = bridge->type_func(ret_type, args_type, varg);
        return this->type(handle);
    }

    bool omis_module_t::equals_type(omis_type_t* a, omis_type_t* b) {
        return a == b || a->get_handle() == b->get_handle();
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

    omis_value_t* omis_module_t::value_integer(u64_t val, u32_t bits) {
        auto type = this->type_i64();
        if (bits == 32)
            type = this->type_i32();
        auto handle = bridge->value_integer(val, bits);
        return this->value(type, handle);
    }

    omis_value_t* omis_module_t::value_float(double val) {
        auto type = this->type_f64();
        auto handle = bridge->value_float(val);
        return this->value(type, handle);
    }

    omis_value_t* omis_module_t::value_bool(bool val) {
        auto type = this->type_bool();
        auto handle = bridge->value_bool(val);
        return this->value(type, handle);
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
        auto handle = bridge->value_func(name, type->get_handle());
        auto value = this->value(type, handle);
        return dynamic_cast<omis_func_t*>(value);
    }
}

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

    omis_type_symbol_t* omis_scope_t::get_type_symbol(predicate_t<omis_type_symbol_t> predicate, bool lookup) {
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

    omis_value_symbol_t* omis_scope_t::get_value_symbol(predicate_t<omis_value_symbol_t> predicate, bool lookup) {
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

    omis_func_t::omis_func_t(omis_module_t *module, omis_type_t *type, void *handle)
            : omis_value_t(module, type, handle) {

    }

    omis_func_t::~omis_func_t() {

    }

    omis_value_t* omis_func_t::create_block(const String &name) {
        auto bridge = module->get_bridge();
        auto ret = bridge->create_block(this->get_handle(), name);
        return module->value(module->type_void(), ret);
    }

    void omis_func_t::activate_block(omis_value_t* block) {
        auto bridge = module->get_bridge();
        bridge->activate_block(block->get_handle());
    }

    omis_value_t* omis_func_t::load(omis_value_t *ptr) {
        auto bridge = module->get_bridge();
        auto ret = bridge->load(ptr->get_handle());
        auto type = bridge->get_value_type(ret);
        return module->value(module->type(type), ret);
    }

    omis_value_t* omis_func_t::store(omis_type_t *ptr, omis_value_t *val) {
        auto bridge = module->get_bridge();
        auto ret = bridge->store(ptr->get_handle(), val->get_handle());
        return module->value(module->type_void(), ret);
    }

    omis_value_t* omis_func_t::neg(omis_value_t *a) {
        auto bridge = module->get_bridge();
        auto ret = bridge->neg(a->get_handle());
        return module->value(a->get_type(), ret);
    }

    omis_value_t* omis_func_t::add(omis_value_t *a, omis_value_t *b) {
        auto bridge = module->get_bridge();
        auto ret = bridge->add(a->get_handle(), b->get_handle());
        auto type = bridge->get_value_type(ret);
        return module->value(module->type(type), ret);
    }

    omis_value_t* omis_func_t::sub(omis_value_t *a, omis_value_t *b) {
        auto bridge = module->get_bridge();
        auto ret = bridge->sub(a->get_handle(), b->get_handle());
        auto type = bridge->get_value_type(ret);
        return module->value(module->type(type), ret);
    }

    omis_value_t* omis_func_t::mul(omis_value_t *a, omis_value_t *b) {
        auto bridge = module->get_bridge();
        auto ret = bridge->mul(a->get_handle(), b->get_handle());
        auto type = bridge->get_value_type(ret);
        return module->value(module->type(type), ret);
    }

    omis_value_t* omis_func_t::div(omis_value_t *a, omis_value_t *b) {
        auto bridge = module->get_bridge();
        auto ret = bridge->div(a->get_handle(), b->get_handle());
        auto type = bridge->get_value_type(ret);
        return module->value(module->type(type), ret);
    }

    omis_value_t* omis_func_t::mod(omis_value_t *a, omis_value_t *b) {
        auto bridge = module->get_bridge();
        auto ret = bridge->mod(a->get_handle(), b->get_handle());
        auto type = bridge->get_value_type(ret);
        return module->value(module->type(type), ret);
    }

    omis_value_t* omis_func_t::eq(omis_value_t *a, omis_value_t *b) {
        auto bridge = module->get_bridge();
        auto ret = bridge->eq(a->get_handle(), b->get_handle());
        return module->value(module->type_bool(), ret);
    }

    omis_value_t* omis_func_t::ne(omis_value_t *a, omis_value_t *b) {
        auto bridge = module->get_bridge();
        auto ret = bridge->ne(a->get_handle(), b->get_handle());
        return module->value(module->type_bool(), ret);
    }

    omis_value_t* omis_func_t::gt(omis_value_t *a, omis_value_t *b) {
        auto bridge = module->get_bridge();
        auto ret = bridge->gt(a->get_handle(), b->get_handle());
        return module->value(module->type_bool(), ret);
    }

    omis_value_t* omis_func_t::ge(omis_value_t *a, omis_value_t *b) {
        auto bridge = module->get_bridge();
        auto ret = bridge->ge(a->get_handle(), b->get_handle());
        return module->value(module->type_bool(), ret);
    }

    omis_value_t* omis_func_t::lt(omis_value_t *a, omis_value_t *b) {
        auto bridge = module->get_bridge();
        auto ret = bridge->lt(a->get_handle(), b->get_handle());
        return module->value(module->type_bool(), ret);
    }

    omis_value_t* omis_func_t::le(omis_value_t *a, omis_value_t *b) {
        auto bridge = module->get_bridge();
        auto ret = bridge->le(a->get_handle(), b->get_handle());
        return module->value(module->type_bool(), ret);
    }

    omis_value_t* omis_func_t::l_not(omis_value_t *a) {
        auto bridge = module->get_bridge();
        auto ret = bridge->l_not(a->get_handle());
        return module->value(module->type_bool(), ret);
    }

    omis_value_t* omis_func_t::l_and(omis_value_t *a, omis_value_t *b) {
        auto bridge = module->get_bridge();
        auto ret = bridge->l_and(a->get_handle(), b->get_handle());
        return module->value(module->type_bool(), ret);
    }

    omis_value_t* omis_func_t::l_or(omis_value_t *a, omis_value_t *b) {
        auto bridge = module->get_bridge();
        auto ret = bridge->l_or(a->get_handle(), b->get_handle());
        return module->value(module->type_bool(), ret);
    }

    omis_value_t* omis_func_t::b_flip(omis_value_t *a) {
        auto bridge = module->get_bridge();
        auto ret = bridge->b_flip(a->get_handle());
        return module->value(a->get_type(), ret);
    }

    omis_value_t* omis_func_t::b_and(omis_value_t *a, omis_value_t *b) {
        auto bridge = module->get_bridge();
        auto ret = bridge->b_and(a->get_handle(), b->get_handle());
        return module->value(a->get_type(), ret);
    }

    omis_value_t* omis_func_t::b_or(omis_value_t *a, omis_value_t *b) {
        auto bridge = module->get_bridge();
        auto ret = bridge->b_or(a->get_handle(), b->get_handle());
        return module->value(a->get_type(), ret);
    }

    omis_value_t* omis_func_t::b_xor(omis_value_t *a, omis_value_t *b) {
        auto bridge = module->get_bridge();
        auto ret = bridge->b_xor(a->get_handle(), b->get_handle());
        return module->value(a->get_type(), ret);
    }

    omis_value_t* omis_func_t::b_shl(omis_value_t *a, omis_value_t *b) {
        auto bridge = module->get_bridge();
        auto ret = bridge->b_shl(a->get_handle(), b->get_handle());
        return module->value(a->get_type(), ret);
    }

    omis_value_t* omis_func_t::b_shr(omis_value_t *a, omis_value_t *b) {
        auto bridge = module->get_bridge();
        auto ret = bridge->b_shr(a->get_handle(), b->get_handle());
        return module->value(a->get_type(), ret);
    }

    omis_value_t* omis_func_t::jump(omis_value_t *pos) {
        auto bridge = module->get_bridge();
        auto ret = bridge->jump(pos->get_handle());
        auto type = bridge->get_value_type(ret);
        return module->value(module->type(type), ret);
    }

    omis_value_t* omis_func_t::jump_cond(omis_value_t* cond, omis_value_t* branch_true, omis_value_t* branch_false) {
        auto bridge = module->get_bridge();
        auto ret = bridge->jump_cond(cond->get_handle(), branch_true->get_handle(), branch_false->get_handle());
        auto type = bridge->get_value_type(ret);
        return module->value(module->type(type), ret);
    }

    omis_value_t* omis_func_t::phi(omis_type_t *type, const std::map<omis_value_t *, omis_value_t *>& incomings) {
        auto bridge = module->get_bridge();
        std::map<omis_handle_t, omis_handle_t> incomings_handles;
        for(auto& pair : incomings) {
            incomings_handles.insert(std::make_pair(pair.first->get_handle(), pair.second->get_handle()));
        }
        auto phi = bridge->phi(type->get_handle(), incomings_handles);
        auto phi_type = bridge->get_value_type(phi);
        return module->value(module->type(phi_type), phi);
    }

    omis_value_t* omis_func_t::create_local_symbol(const String &name, omis_type_t *type, omis_value_t *value) {
        auto bridge = module->get_bridge();
        auto symbol = bridge->alloc(type->get_handle());
        auto ret = bridge->store(symbol, value->get_handle());
        auto ret_type = bridge->get_value_type(ret);
        return module->value(module->type(ret_type), ret);
    }
}