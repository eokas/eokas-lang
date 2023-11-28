#ifndef _EOKAS_LLVM_ENGINE_H_
#define _EOKAS_LLVM_ENGINE_H_

#include "../omis/model.h"

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
    omis_bridge_t* llvm_bridge_init(const String& name);
    void llvm_bridge_quit(omis_bridge_t* bridge);

	bool llvm_jit(omis_module_t* module);
	bool llvm_aot(omis_module_t* module);
}

#endif//_EOKAS_LLVM_ENGINE_H_
