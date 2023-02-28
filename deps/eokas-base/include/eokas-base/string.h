
#ifndef  _EOKAS_BASE_STRING_H_
#define  _EOKAS_BASE_STRING_H_

#include "header.h"
#include <sstream>
#include <cstdarg>

_BeginNamespace(eokas)

typedef std::list<String> StringList;
typedef std::vector<String> StringVector;
typedef std::map<String, String> StringMap;

#ifndef _STRING_LITTLE_LENGTH
#define _STRING_LITTLE_LENGTH 16
#endif//_STRING_LITTLE_LENGTH
#ifndef _STRING_MIDDLE_LENGTH
#define _STRING_MIDDLE_LENGTH 256
#endif//_STRING_MIDDLE_LENGTH

#ifndef _FormatVA
#define _FormatVA(ret, fmt) \
{ \
		va_list ap = {0}; \
    \
    va_start(ap, fmt); \
    i32_t size = vsnprintf(nullptr, 0, fmt, ap); \
    va_end(ap); \
    \
    char* buff = new char[size + 1]; \
    \
    va_start(ap, fmt); \
    vsnprintf(buff, size + 1, fmt, ap); \
    buff[size] = '\0'; \
    va_end(ap); \
    \
    ret = String(buff); \
    delete[]buff; \
}
#endif//_FormatVA

class String
{
public:
  static const size_t npos;
  static const String empty;
  static const String zero;
  static const String one;
  static const String trueValue;
  static const String falseValue;

  /*
  ============================================================================================
  ==== class functions
  ============================================================================================
  */
  static char measure(size_t len);
  static size_t predict(size_t len);
  static MBString unicodeToUtf8(const WCString& unicodeStr, bool bom);
  static WCString utf8ToUnicode(const MBString& utf8Str, bool bom);
  static String encodeURL(const String& str);
  static String decodeURL(const String& str);
  static String format(const char* fmt, ...);
  static String repeat(const String& str, size_t n);
  static String join(const StringVector& segments, const String& delim);
  static String join(const StringMap& segments, const String& conn, const String& delim);

  /*
  ============================================================================================
  ==== type casting functions
  ============================================================================================
  */
  template<typename T>
  inline static String valueToString(const T& value);

  template<typename T>
  inline static T stringToValue(const String& str);

  template<typename T>
  inline static T binstrToValue(const String& str);

  template<typename T>
  inline static T hexstrToValue(const String& str);

public:
  String(char chr = '\0', size_t len = 1);
  String(const char* mbcstr, size_t len = npos);
  String(const MBString& mbstr);
  String(const WCString& wcstr);
  String(const String& other);
  String(String&& other);
  virtual ~String();

public:
  String& operator=(const String& rhs);
  String& operator=(String&& rhs);
  String& operator+=(const String& rhs);
  String operator+(const String& rhs) const;
  bool operator>(const String& rhs) const;
  bool operator<(const String& rhs) const;
  bool operator>=(const String& rhs) const;
  bool operator<=(const String& rhs) const;
  bool operator==(const String& rhs) const;
  bool operator!=(const String& rhs) const;

public:
  String& clear();
  String& append(const String& str);
  String& insert(size_t pos, const String& str);
  String& remove(size_t pos, size_t len);

  size_t length() const;
  const char* cstr() const;
  bool isEmpty() const;
  char at(size_t index) const;
  size_t find(char chr, size_t pos = 0) const;
  size_t find(const String& str, size_t pos = 0) const;
  size_t rfind(char chr) const;
  size_t rfind(const String& str) const;

  bool contains(const String& str) const;
  bool containsOne(const StringVector& strs) const;
  bool containsAll(const StringVector& strs) const;
  bool startsWith(const String& str) const;
  bool startsWith(const StringVector& strs) const;
  bool endsWith(const String& str) const;
  bool endsWith(const StringVector& strs) const;
  String toUpper() const;
  String toLower() const;
  String reverse() const;
  String substr(size_t pos, size_t len = npos) const;
  String left(size_t len) const;
  String right(size_t len) const;
  String replace(const String& str1, const String& str2) const;
  String replace(const StringMap& nameValues) const;
  String trim(bool left = true, bool right = true) const;
  StringVector split(const String& delim) const;

private:
  char* mData;
  size_t mSize;
  size_t mCapacity;
  i32_t mMetric;
  char mValue[_STRING_LITTLE_LENGTH];
};
/*
============================================================================================
==== template implementations for class String
============================================================================================
*/
template<typename T>
inline String String::valueToString(const T& value)
{
  std::stringstream stream;
  stream << value;
  return stream.str();
}
template<>
inline String String::valueToString<bool>(const bool& value)
{
  return value ? "true" : "false";
}

template<typename T>
inline T String::stringToValue(const String& str)
{
  T value = 0;
  std::stringstream stream(str.mData);
  stream >> value;
  return value;
}
template<>
inline bool String::stringToValue<bool>(const String& str)
{
  return (str == "true" || str == "TRUE" || str == "True" || str == "1");
}

template<typename T>
inline T String::binstrToValue(const String& str)
{
  size_t len = str.length();
  size_t pos = 0;

  if (str.at(0) == '0' && (str.at(1) == 'b' || str.at(1) == 'B'))
    pos = 2;

  if (len - pos > sizeof(T) * 8)
    return 0;

  u64_t result = 0;
  while (pos < len)
  {
    int bit = str.at(pos) - 48;
    result |= bit << (len - pos - 1);
    pos++;
  }
  return static_cast<T>(result);
}

template<typename T>
inline T String::hexstrToValue(const String& str)
{
  size_t len = str.length();
  size_t pos = 0;

  if (str.at(0) == '0' && (str.at(1) == 'x' || str.at(1) == 'X'))
    pos = 2;

  if (len - pos > sizeof(T) * 2) // sizeof(T) * 8 / 4
    return 0;

  u64_t result = 0;
  while (pos < len)
  {
    char c = str.at(pos);

    int bit = 0;
    if (c >= '0' && c <= '9')
      bit = c - '0';
    else if (c >= 'A' && c <= 'F')
      bit = c - 'A' + 10;
    else if (c >= 'a' && c <= 'f')
      bit = c - 'a' + 10;

    result |= bit << (len - pos - 1) * 4;

    pos++;
  }
  return static_cast<T>(result);
}

/*
============================================================================================
==== StringValue -- type casting class
============================================================================================
*/
class StringValue
{
public:
  static const StringValue empty;
  static const StringValue zero;
  static const StringValue one;
  static const StringValue trueValue;
  static const StringValue falseValue;

public:
  StringValue(const StringValue& var);
  StringValue(const String& value = "");
  StringValue(const char* value);
  StringValue(i8_t value);
  StringValue(u8_t value);
  StringValue(i16_t value);
  StringValue(u16_t value);
  StringValue(i32_t value);
  StringValue(u32_t value);
  StringValue(i64_t value);
  StringValue(u64_t value);
  StringValue(f32_t value);
  StringValue(f64_t value);
  StringValue(bool value);

public:
  StringValue& operator=(const StringValue& var);
  StringValue& operator=(const String& value);
  StringValue& operator=(const char* value);
  StringValue& operator=(i8_t value);
  StringValue& operator=(u8_t value);
  StringValue& operator=(i16_t value);
  StringValue& operator=(u16_t value);
  StringValue& operator=(i32_t value);
  StringValue& operator=(u32_t value);
  StringValue& operator=(i64_t value);
  StringValue& operator=(u64_t value);
  StringValue& operator=(f32_t value);
  StringValue& operator=(f64_t value);
  StringValue& operator=(bool value);

  operator const String& () const;
  operator i8_t() const;
  operator u8_t() const;
  operator i16_t() const;
  operator u16_t() const;
  operator i32_t() const;
  operator u32_t() const;
  operator i64_t() const;
  operator u64_t() const;
  operator f32_t() const;
  operator f64_t() const;
  operator bool() const;

  bool operator==(const StringValue& rhs) const;
  bool operator!=(const StringValue& rhs) const;
  bool operator==(const String& rhs) const;
  bool operator!=(const String& rhs) const;

public:
  const String& string() const
  {
    return (const String&)(*this);
  }

  template<typename T>
  T value() const
  {
    return (T)(*this);
  }

private:
  String mValue;
};

using StringValueList = std::list<StringValue>;
using StringValueVector = std::vector<StringValue>;
using StringValueMap = std::map<String, StringValue>;

_EndNamespace(eokas)

#endif//_EOKAS_BASE_STRING_H_
