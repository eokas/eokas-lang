#include "./model.h"
#include "./scope.h"
#include "./bridge.h"

namespace eokas {
    omis_type_t::omis_type_t(omis_module_t *module, omis_handle_t handle)
        : module(module), handle(handle), default_value(nullptr) {

    }

    omis_type_t::~omis_type_t() {

    }

    omis_module_t* omis_type_t::get_module() {
        return module;
    }

    omis_typeid_t omis_type_t::get_typeid() {
        return id;
    }

    omis_handle_t omis_type_t::get_handle() {
        return handle;
    }

    omis_value_t* omis_type_t::get_default_value() {
        return default_value;
    }

    omis_struct_t::omis_struct_t(omis_module_t *module, omis_typeid_t id, void *handle)
        : omis_type_t(module, id, handle) {

    }

    omis_struct_t::~omis_struct_t() {
        members.clear();
    }

    bool omis_struct_t::extends(const String& base) {
        auto symbol = module->get_scope()->get_type_symbol(base, true);
        if(symbol == nullptr)
            return false;
        auto base_type = symbol->type;
        auto base_struct = dynamic_cast<omis_struct_t*>(base_type);
        if(base_struct == nullptr)
            return false;
        return this->extends(base_struct);
    }

    bool omis_struct_t::extends(omis_struct_t* base) {
        if (base == nullptr)
            return false;
        this->add_member("base", base);
        for (auto& m: base->members) {
            if(m.name == "base")
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
        auto ret = bridge->create_block(name);
        return module->create_value(ret);
    }

    void omis_func_t::activate_block(omis_value_t* block) {
        auto bridge = module->get_bridge();
        bridge->activate_block(block->get_handle());
    }

    omis_value_t* omis_func_t::load(omis_value_t *ptr) {
        auto bridge = module->get_bridge();
        auto ret = bridge->load(ptr->get_handle());
        return module->create_value(ret);
    }

    void omis_func_t::store(omis_type_t *ptr, omis_value_t *val) {
        auto bridge = module->get_bridge();
        bridge->store(ptr->get_handle(), val->get_handle());
    }

    omis_value_t* omis_func_t::neg(omis_value_t *a) {
        auto bridge = module->get_bridge();
        auto ret = bridge->neg(a->get_handle());
        return module->create_value(ret);
    }

    omis_value_t* omis_func_t::add(omis_value_t *a, omis_value_t *b) {
        auto bridge = module->get_bridge();
        auto ret = bridge->add(a->get_handle(), b->get_handle());
        return module->create_value(ret);
    }

    omis_value_t* omis_func_t::sub(omis_value_t *a, omis_value_t *b) {
        auto bridge = module->get_bridge();
        auto ret = bridge->sub(a->get_handle(), b->get_handle());
        return module->create_value(ret);
    }

    omis_value_t* omis_func_t::mul(omis_value_t *a, omis_value_t *b) {
        auto bridge = module->get_bridge();
        auto ret = bridge->mul(a->get_handle(), b->get_handle());
        return module->create_value(ret);
    }

    omis_value_t* omis_func_t::div(omis_value_t *a, omis_value_t *b) {
        auto bridge = module->get_bridge();
        auto ret = bridge->div(a->get_handle(), b->get_handle());
        return module->create_value(ret);
    }

    omis_value_t* omis_func_t::mod(omis_value_t *a, omis_value_t *b) {
        auto bridge = module->get_bridge();
        auto ret = bridge->mod(a->get_handle(), b->get_handle());
        return module->create_value(ret);
    }

    omis_value_t* omis_func_t::cmp(omis_value_t *a, omis_value_t *b) {
        auto bridge = module->get_bridge();
        auto ret = bridge->cmp(a->get_handle(), b->get_handle());
        return module->create_value(ret);
    }

    omis_value_t* omis_func_t::eq(omis_value_t *a, omis_value_t *b) {
        auto bridge = module->get_bridge();
        auto ret = bridge->eq(a->get_handle(), b->get_handle());
        return module->create_value(ret);
    }

    omis_value_t* omis_func_t::ne(omis_value_t *a, omis_value_t *b) {
        auto bridge = module->get_bridge();
        auto ret = bridge->ne(a->get_handle(), b->get_handle());
        return module->create_value(ret);
    }

    omis_value_t* omis_func_t::gt(omis_value_t *a, omis_value_t *b) {
        auto bridge = module->get_bridge();
        auto ret = bridge->gt(a->get_handle(), b->get_handle());
        return module->create_value(ret);
    }

    omis_value_t* omis_func_t::ge(omis_value_t *a, omis_value_t *b) {
        auto bridge = module->get_bridge();
        auto ret = bridge->ge(a->get_handle(), b->get_handle());
        return module->create_value(ret);
    }

    omis_value_t* omis_func_t::lt(omis_value_t *a, omis_value_t *b) {
        auto bridge = module->get_bridge();
        auto ret = bridge->lt(a->get_handle(), b->get_handle());
        return module->create_value(ret);
    }

    omis_value_t* omis_func_t::le(omis_value_t *a, omis_value_t *b) {
        auto bridge = module->get_bridge();
        auto ret = bridge->le(a->get_handle(), b->get_handle());
        return module->create_value(ret);
    }

    omis_value_t* omis_func_t::l_not(omis_value_t *a) {
        auto bridge = module->get_bridge();
        auto ret = bridge->l_not(a->get_handle());
        return module->create_value(ret);
    }

    omis_value_t* omis_func_t::l_and(omis_value_t *a, omis_value_t *b) {
        auto bridge = module->get_bridge();
        auto ret = bridge->l_and(a->get_handle(), b->get_handle());
        return module->create_value(ret);
    }

    omis_value_t* omis_func_t::l_or(omis_value_t *a, omis_value_t *b) {
        auto bridge = module->get_bridge();
        auto ret = bridge->l_or(a->get_handle(), b->get_handle());
        return module->create_value(ret);
    }

    omis_value_t* omis_func_t::b_flip(omis_value_t *a) {
        auto bridge = module->get_bridge();
        auto ret = bridge->b_flip(a->get_handle());
        return module->create_value(ret);
    }

    omis_value_t* omis_func_t::b_and(omis_value_t *a, omis_value_t *b) {
        auto bridge = module->get_bridge();
        auto ret = bridge->b_and(a->get_handle(), b->get_handle());
        return module->create_value(ret);
    }

    omis_value_t* omis_func_t::b_or(omis_value_t *a, omis_value_t *b) {
        auto bridge = module->get_bridge();
        auto ret = bridge->b_or(a->get_handle(), b->get_handle());
        return module->create_value(ret);
    }

    omis_value_t* omis_func_t::b_xor(omis_value_t *a, omis_value_t *b) {
        auto bridge = module->get_bridge();
        auto ret = bridge->b_xor(a->get_handle(), b->get_handle());
        return module->create_value(ret);
    }

    omis_value_t* omis_func_t::b_shl(omis_value_t *a, omis_value_t *b) {
        auto bridge = module->get_bridge();
        auto ret = bridge->b_shl(a->get_handle(), b->get_handle());
        return module->create_value(ret);
    }

    omis_value_t* omis_func_t::b_shr(omis_value_t *a, omis_value_t *b) {
        auto bridge = module->get_bridge();
        auto ret = bridge->b_shr(a->get_handle(), b->get_handle());
        return module->create_value(ret);
    }

    omis_value_t* omis_func_t::jump(omis_value_t *pos) {
        auto bridge = module->get_bridge();
        auto ret = bridge->jump(pos->get_handle());
        return module->create_value(ret);
    }

    omis_value_t* omis_func_t::jump_cond(omis_value_t* cond, omis_value_t* branch_true, omis_value_t* branch_false) {
        auto bridge = module->get_bridge();
        auto ret = bridge->jump_cond(cond->get_handle(), branch_true->get_handle(), branch_false->get_handle());
        return module->create_value(ret);
    }

    omis_value_t* omis_func_t::phi(omis_type_t *type, const std::map<omis_value_t *, omis_value_t *> incommings) {
        auto bridge = module->get_bridge();
        auto phi = bridge->phi(type->get_handle(), incommings.size());
        for(auto& pair : incommings) {
            bridge->phi_set_incomming(phi,pair.first, pair.second);
        }
        return module->create_value(phi);
    }

    omis_value_t* omis_func_t::create_local_symbol(const String &name, omis_type_t *type, omis_value_t *value) {
        auto bridge = module->get_bridge();
        auto symbol = bridge->new_value(type->get_handle());
        bridge->store(symbol, value->get_handle());
        return module->create_value(symbol);
    }

    omis_module_t::omis_module_t(const String &name, omis_bridge_t *bridge)
        : name(name)
        , bridge(bridge)
        , root(new omis_scope_t(nullptr, nullptr))
        , scope(this->root)
        , usings()
        , types()
        , values() {

    }

    omis_module_t::~omis_module_t() {
        _DeletePointer(root);
        _DeleteList(types);
        _DeleteList(values);
    }

    omis_bridge_t* omis_module_t::get_bridge() {
        return bridge;
    }

    omis_scope_t* omis_module_t::get_scope() {
        return scope;
    }

    omis_scope_t *omis_module_t::push_scope(omis_func_t* func) {
        this->scope = this->scope->add_child(func);
    }

    omis_scope_t *omis_module_t::pop_scope() {
        if(this->scope == this->root)
            return this->root;
        this->scope = this->scope->parent;
    }

    bool omis_module_t::using_module(omis_module_t *other) {
        auto iter = std::find(usings.begin(), usings.end(), other);
        if(iter != usings.end())
            return false;
        usings.push_back(other);
        return true;
    }

    omis_type_t *omis_module_t::create_type(omis_handle_t handle) {
        auto type = new omis_type_t(this, handle);
        this->types.push_back(type);
        return type;
    }

    bool omis_module_t::equals_type(omis_type_t *a, omis_type_t *b) {
        return a == b || a->get_handle() == b->get_handle();
    }

    bool omis_module_t::can_losslessly_bitcast(omis_type_t *a, omis_type_t *b) {
        return bridge->can_losslessly_cast(a->get_handle(), b->get_handle());
    }

    omis_type_t* omis_module_t::type_void() {
        static omis_type_t* type = this->create_type(bridge->type_void());
        return type;
    }

    omis_type_t* omis_module_t::type_i8() {
        static omis_type_t* type = this->create_type(bridge->type_i8());
        return type;
    }

    omis_type_t* omis_module_t::type_i16() {
        static omis_type_t* type = this->create_type(bridge->type_i16());
        return type;
    }

    omis_type_t* omis_module_t::type_i32() {
        static omis_type_t* type = this->create_type(bridge->type_i32());
        return type;
    }

    omis_type_t* omis_module_t::type_i64() {
        static omis_type_t* type = this->create_type(bridge->type_i64());
        return type;
    }

    omis_type_t* omis_module_t::type_f32() {
        static omis_type_t* type = this->create_type(bridge->type_f32());
        return type;
    }

    omis_type_t *omis_module_t::type_f64() {
        static omis_type_t* type = this->create_type(bridge->type_f64());
        return type;
    }

    omis_type_t *omis_module_t::type_bool() {
        static omis_type_t* type = this->create_type(bridge->type_bool());
        return type;
    }

    omis_value_t* omis_module_t::create_value(omis_type_t *type, omis_handle_t handle) {
        auto val = new omis_value_t(this, type, handle);
        this->values.push_back(val);
        return val;
    }

    omis_value_t *omis_module_t::constant_integer(u64_t val, u32_t bits) {
        auto type = this->type_i64();
        if(bits == 32)
            type = this->type_i32();
        auto handle = bridge->constant_integer(val, bits);
        return this->create_value(type, handle);
    }

    omis_value_t* omis_module_t::constant_float(double val) {
        auto type = this->type_f64();
        auto handle = bridge->constant_float(val);
        return this->create_value(type, handle);
    }

    omis_value_t *omis_module_t::constant_bool(bool val) {
        auto type = this->type_bool();
        auto handle = bridge->constant_bool(val);
        return this->create_value(type, handle);
    }
}
