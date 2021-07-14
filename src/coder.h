#ifndef _EOKAS_CODER_H_
#define _EOKAS_CODER_H_

#include  "header.h"

_BeginNamespace(eokas)

class coder_t 
{
public:
    coder_t(Stream& stream);
    virtual ~coder_t();

public:
    bool encode(struct ast_module_t* node);

private:
    struct coder_llvm_t* impl;
};

_EndNamespace(eokas)

#endif//_EOKAS_CODER_H_
