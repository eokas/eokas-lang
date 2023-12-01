#ifndef _EOKAS_LLVM_ENGINE_H_
#define _EOKAS_LLVM_ENGINE_H_

#include "../model.h"

namespace llvm
{
    class LLVMContext;
    class Type;
    class Value;
    class Function;
    class Module;
    class BasicBlock;
}

namespace eokas
{
    omis_bridge_t* llvm_init();
    void llvm_quit(omis_bridge_t* bridge);
}

#endif//_EOKAS_LLVM_ENGINE_H_
