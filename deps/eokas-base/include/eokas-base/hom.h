
#ifndef  _EOKAS_BASE_HOM_H_
#define  _EOKAS_BASE_HOM_H_

/**
 * HOM: Hierarchical Object Model.
 * It is similar with DOM (Document Object Model)
 * Widely used in JSON
 * */

#include <utility>

#include "string.h"

_BeginNamespace(eokas)

enum class HomType
{
    Null,
    Number,
    Boolean,
    String,
    Array,
    Object,
};

struct HomValue
{
    HomType type;

    explicit HomValue(HomType type)
        :type(type)
    {}
};

using HomValueRef = std::shared_ptr<HomValue>;
using HomValueArray = std::vector<HomValueRef>;
using HomValueMap = std::map<String, HomValueRef>;

struct HomNull : public HomValue
{
    explicit HomNull()
        : HomValue(HomType::Null)
    {}

    static HomValueRef make()
    {
        return HomValueRef(new HomNull());
    }
};

struct HomNumber : public HomValue
{
    f64_t value;

    explicit HomNumber(f64_t value = 0)
        : HomValue(HomType::Number), value(value)
    {}

    static HomValueRef make(f64_t value)
    {
        return HomValueRef(new HomNumber(value));
    }

    static f64_t pick(const HomValueRef& value)
    {
        auto ptr = (HomNumber*)value.get();
        return ptr != nullptr ? ptr->value : 0;
    }
};

struct HomBoolean : public HomValue
{
    bool value;

    explicit HomBoolean(bool value = false)
        : HomValue(HomType::Boolean), value(value)
    {}

    static HomValueRef make(bool value)
    {
        return HomValueRef(new HomBoolean(value));
    }

    static bool pick(const HomValueRef& value)
    {
        auto ptr = (HomBoolean*)value.get();
        return ptr != nullptr && ptr->value;
    }
};

struct HomString : public HomValue
{
    String value;

    explicit HomString(const String& value)
        : HomValue(HomType::String), value(std::move(value))
    { }

    static HomValueRef make(const String& value)
    {
        return HomValueRef(new HomString(value));
    }

    static String pick(const HomValueRef& value)
    {
        auto ptr = (HomString*)value.get();
        return ptr != nullptr ? ptr->value : "";
    }
};

struct HomArray : public HomValue
{
    HomValueArray value;

    explicit HomArray(const HomValueArray& val = {});

    HomValueRef& operator[](size_t index);

    f64_t getNumber(size_t index, f64_t defaultValue = 0);
    bool getBoolean(size_t index, bool defaultValue = false);
    String getString(size_t index, const String& defaultValue = "");
    HomValueArray getArray(size_t index, const HomValueArray& defaultValue = {});
    HomValueMap getObject(size_t index, const HomValueMap& defaultValue = {});

    bool get(size_t index, HomValueRef& val);
    bool get(size_t index, HomType type, HomValueRef& val);
    bool get(size_t index, f64_t& val);
    bool get(size_t index, bool& val);
    bool get(size_t index, String& val);
    bool get(size_t index, HomValueArray& val);
    bool get(size_t index, HomValueMap& val);

    void set(size_t index, HomValueRef val);
    void set(size_t index, f64_t val);
    void set(size_t index, bool val);
    void set(size_t index, const String& val);
    void set(size_t index, const HomValueArray& val);
    void set(size_t index, const HomValueMap& val);

    HomArray& add(HomValueRef val);
    HomArray& add(f64_t val);
    HomArray& add(bool val);
    HomArray& add(const String& val);
    HomArray& add(const HomValueArray& val);
    HomArray& add(const HomValueMap& val);

    static HomValueRef make(const HomValueArray& value);
    static HomValueArray pick(const HomValueRef& value);
};

struct HomObject : public HomValue
{
    HomValueMap value;

    explicit HomObject(const HomValueMap& value = {});

    HomValueRef& operator[](const String& key);

    HomValueRef& getValue(const String& key);
    f64_t getNumber(const String& key, f64_t defaultValue = 0);
    bool getBoolean(const String& key, bool defaultValue = false);
    String getString(const String& key, const String& defaultValue = "");
    HomValueArray getArray(const String& key, const HomValueArray& defaultValue = {});
    HomValueMap getObject(const String& key, const HomValueMap& defaultValue = {});

    bool get(const String& key, HomValueRef& val);
    bool get(const String& key, HomType type, HomValueRef& val);
    bool get(const String& key, f64_t& val);
    bool get(const String& key, bool& val);
    bool get(const String& key, String& val);
    bool get(const String& key, HomValueArray& val);
    bool get(const String& key, HomValueMap& val);

    void set(const String& key, HomValueRef val);
    void set(const String& key, f64_t val);
    void set(const String& key, bool val);
    void set(const String& key, const String& val);
    void set(const String& key, const HomValueArray& val);
    void set(const String& key, const HomValueMap& val);

    static HomValueRef make(const HomValueMap& value);
    static HomValueMap pick(const HomValueRef& value);
};

class HOM
{
public:
    HOM()
    { }

    virtual ~HOM()
    {
        this->clear();
    }

    template<typename T>
    T* make()
    {
        T* x = new T();
        mValues.push_back(x);
        return x;
    }

    template<typename T>
    T* make(const typename T::value_type& value)
    {
        T* x = new T();
        x->value = value;
        mValues.push_back(x);
        return x;
    }

    void drop(HomValue* value)
    {
        auto iter = mValues.begin();
        while(iter != mValues.end())
        {
            HomValue* val = *iter;
            if(val != value)
                continue;
            _DeletePointer(val);
            iter = mValues.erase(iter);
        }
    }

    void clear()
    {
        _DeleteList(mValues);
    }

private:
    std::vector<HomValue*> mValues = {};
};

_EndNamespace(eokas)

#endif//_EOKAS_BASE_HOM_H_
