
#ifndef  _EOKAS_BASE_JSON_H_
#define  _EOKAS_BASE_JSON_H_

#include "hom.h"

_BeginNamespace(eokas)

struct JSON
{
    static String stringify(const HomValueRef& json);

    static HomValueRef parse(const String& source);
};

_EndNamespace(eokas)

#endif//_EOKAS_BASE_JSON_H_
