#include "./model.h"
#include "./scope.h"
#include "./bridge.h"

namespace eokas {
    omis_type_t::omis_type_t(eokas::omis_module_t *module, omis_typeid_t id, omis_handle_t handle)
        : module(module), id(id), handle(handle), default_value(nullptr) {

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

    omis_struct_t::omis_struct_t(eokas::omis_module_t *module, omis_typeid_t id, void *handle)
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

    omis_value_t::omis_value_t(eokas::omis_module_t *module, eokas::omis_type_t *type, void *handle)
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

    omis_func_t::omis_func_t(eokas::omis_module_t *module, eokas::omis_type_t *type, void *handle)
        : omis_value_t(module, type, handle) {

    }

    omis_func_t::~omis_func_t() {

    }

    omis_value_t* omis_func_t::load(eokas::omis_value_t *ptr) {
        auto bridge = module->get_bridge();
        auto ret = bridge->load(ptr->get_handle());
        return module->create_value(ret);
    }

    void omis_func_t::store(eokas::omis_type_t *ptr, eokas::omis_value_t *val) {
        auto bridge = module->get_bridge();
        bridge->store(ptr->get_handle(), val->get_handle());
    }

    omis_value_t* omis_func_t::neg(eokas::omis_value_t *a) {
        auto bridge = module->get_bridge();
        auto ret = bridge->neg(a->get_handle());
        return module->create_value(ret);
    }

    omis_value_t* omis_func_t::add(eokas::omis_value_t *a, eokas::omis_value_t *b) {
        auto bridge = module->get_bridge();
        auto ret = bridge->add(a->get_handle(), b->get_handle());
        return module->create_value(ret);
    }

    omis_value_t* omis_func_t::sub(eokas::omis_value_t *a, eokas::omis_value_t *b) {
        auto bridge = module->get_bridge();
        auto ret = bridge->sub(a->get_handle(), b->get_handle());
        return module->create_value(ret);
    }

    omis_value_t* omis_func_t::mul(eokas::omis_value_t *a, eokas::omis_value_t *b) {
        auto bridge = module->get_bridge();
        auto ret = bridge->mul(a->get_handle(), b->get_handle());
        return module->create_value(ret);
    }

    omis_value_t* omis_func_t::div(eokas::omis_value_t *a, eokas::omis_value_t *b) {
        auto bridge = module->get_bridge();
        auto ret = bridge->div(a->get_handle(), b->get_handle());
        return module->create_value(ret);
    }

    omis_value_t* omis_func_t::mod(eokas::omis_value_t *a, eokas::omis_value_t *b) {
        auto bridge = module->get_bridge();
        auto ret = bridge->mod(a->get_handle(), b->get_handle());
        return module->create_value(ret);
    }

    omis_value_t* omis_func_t::cmp(eokas::omis_value_t *a, eokas::omis_value_t *b) {
        auto bridge = module->get_bridge();
        auto ret = bridge->cmp(a->get_handle(), b->get_handle());
        return module->create_value(ret);
    }

    omis_value_t* omis_func_t::l_not(eokas::omis_value_t *a) {
        auto bridge = module->get_bridge();
        auto ret = bridge->l_not(a->get_handle());
        return module->create_value(ret);
    }

    omis_value_t* omis_func_t::l_and(eokas::omis_value_t *a, eokas::omis_value_t *b) {
        auto bridge = module->get_bridge();
        auto ret = bridge->l_and(a->get_handle(), b->get_handle());
        return module->create_value(ret);
    }

    omis_value_t* omis_func_t::l_or(eokas::omis_value_t *a, eokas::omis_value_t *b) {
        auto bridge = module->get_bridge();
        auto ret = bridge->l_or(a->get_handle(), b->get_handle());
        return module->create_value(ret);
    }

    omis_value_t* omis_func_t::b_flip(eokas::omis_value_t *a) {
        auto bridge = module->get_bridge();
        auto ret = bridge->b_flip(a->get_handle());
        return module->create_value(ret);
    }

    omis_value_t* omis_func_t::b_and(eokas::omis_value_t *a, eokas::omis_value_t *b) {
        auto bridge = module->get_bridge();
        auto ret = bridge->b_and(a->get_handle(), b->get_handle());
        return module->create_value(ret);
    }

    omis_value_t* omis_func_t::b_or(eokas::omis_value_t *a, eokas::omis_value_t *b) {
        auto bridge = module->get_bridge();
        auto ret = bridge->b_or(a->get_handle(), b->get_handle());
        return module->create_value(ret);
    }

    omis_value_t* omis_func_t::b_xor(eokas::omis_value_t *a, eokas::omis_value_t *b) {
        auto bridge = module->get_bridge();
        auto ret = bridge->b_xor(a->get_handle(), b->get_handle());
        return module->create_value(ret);
    }

    omis_value_t* omis_func_t::b_shl(eokas::omis_value_t *a, eokas::omis_value_t *b) {
        auto bridge = module->get_bridge();
        auto ret = bridge->b_shl(a->get_handle(), b->get_handle());
        return module->create_value(ret);
    }

    omis_value_t* omis_func_t::b_shr(eokas::omis_value_t *a, eokas::omis_value_t *b) {
        auto bridge = module->get_bridge();
        auto ret = bridge->b_shr(a->get_handle(), b->get_handle());
        return module->create_value(ret);
    }

    omis_module_t::omis_module_t(const eokas::String &name, eokas::omis_bridge_t *bridge)
        : name(name)
        , bridge(bridge)
        , scope(new omis_scope_t(nullptr, nullptr))
        , usings()
        , types()
        , values() {

    }

    omis_module_t::~omis_module_t() {
        _DeletePointer(scope);
        _DeleteList(types);
        _DeleteList(values);
    }

    omis_bridge_t* omis_module_t::get_bridge() {
        return bridge;
    }

    omis_scope_t* omis_module_t::get_scope() {
        return scope;
    }

    bool omis_module_t::using_module(omis_module_t *other) {
        auto iter = std::find(usings.begin(), usings.end(), other);
        if(iter != usings.end())
            return false;
        usings.push_back(other);
        return true;
    }

    omis_type_t *omis_module_t::create_type(omis_typeid_t id, omis_handle_t handle) {
        omis_type_t* type = nullptr;

    }
}
