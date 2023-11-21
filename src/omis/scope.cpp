#include "./scope.h"

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
                auto* symbol = scope->types.get([&](auto name, auto symbol)->auto {
                    return predicate(symbol);
                });
                if (symbol != nullptr)
                    return symbol;
            }
            return nullptr;
        } else {
            return this->types.get([&](auto name, auto symbol)->auto {
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
                auto* symbol = scope->values.get([&](auto name, auto symbol)->auto {
                    return predicate(symbol);
                });
                if (symbol != nullptr)
                    return symbol;
            }
            return nullptr;
        } else {
            return this->values.get([&](auto name, auto symbol)->auto {
                return predicate(symbol);
            });
        }
    }
}

