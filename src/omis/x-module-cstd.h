
#ifndef _EOKAS_LLVM_MODULE_C_H_
#define _EOKAS_LLVM_MODULE_C_H_

#include "./model.h"

namespace eokas
{
	struct omis_module_cstd_t : omis_module_t
	{
        omis_module_cstd_t(omis_context_t* context);
		
		virtual bool main() override;
		
		void printf();
		void sprintf();
		void malloc();
		void free();
		void strlen();
	};
}

#endif //_EOKAS_LLVM_MODULE_C_H_
