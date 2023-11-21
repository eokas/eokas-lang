
#ifndef _EOKAS_OMIS_SCOPE_H_
#define _EOKAS_OMIS_SCOPE_H_

#include "./header.h"

namespace eokas
{
    template<typename T, bool gc = true>
    struct omis_table_t {
        std::map<String, T *> table;

        explicit omis_table_t() : table() {
        }

        ~omis_table_t() {
            if (gc && !this->table.empty()) {
                _DeleteMap(this->table);
            }
            this->table.clear();
        }

        bool add(const String &name, T *object) {
            if (this->table.find(name) != this->table.end())
                return false;
            this->table.insert(std::make_pair(name, object));
            return true;
        }

        T *get(const String &name) {
            auto iter = this->table.find(name);
            if (iter == this->table.end())
                return nullptr;
            return iter->second;
        }

        T *get(const std::function<bool(const String&, const T&)>& predicate) {
            for(auto& pair : this->table) {
                if(predicate(pair.first, *pair.second)) {
                    return pair.second;
                }
            }
            return nullptr;
        }
    };

    struct omis_value_symbol_t
    {
        String name = "";
        omis_value_t* value = nullptr;
        omis_scope_t* scope = nullptr;
    };

    struct omis_type_symbol_t
    {
        String name = "";
        omis_type_t* type = nullptr;
    };

    struct omis_scope_t
    {
        omis_scope_t* parent;
        omis_func_t* func;
        std::vector<omis_scope_t*> children;

        omis_table_t<omis_type_symbol_t> types;
        omis_table_t<omis_value_symbol_t> values;

        omis_scope_t(omis_scope_t* parent, omis_func_t* func);
        virtual ~omis_scope_t();

        omis_scope_t* add_child(omis_func_t* f = nullptr);

        bool add_type_symbol(const String& name, omis_type_t* type);
        omis_type_symbol_t* get_type_symbol(const String& name, bool lookup);
        omis_type_symbol_t* get_type_symbol(predicate_t<omis_type_symbol_t> predicate, bool lookup);

        bool add_value_symbol(const String& name, omis_value_t* value);
        omis_value_symbol_t* get_value_symbol(const String& name, bool lookup);
        omis_value_symbol_t* get_value_symbol(predicate_t<omis_value_symbol_t> predicate, bool lookup);
    };
}

#endif //_EOKAS_OMIS_SCOPE_H_
