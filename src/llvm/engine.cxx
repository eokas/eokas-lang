#include "engine.h"
#include "coder.h"
#include "models.h"

#include <llvm/ADT/APFloat.h>
#include <llvm/ADT/STLExtras.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Mangler.h>
#include <llvm/IR/LegacyPassManager.h>

#include <llvm/Support/FileSystem.h>
#include <llvm/Support/Host.h>
#include <llvm/Support/TargetRegistry.h>
#include <llvm/Support/TargetSelect.h>

#include <llvm/Target/TargetMachine.h>
#include <llvm/Target/TargetOptions.h>

#include <llvm/ExecutionEngine/MCJIT.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/GenericValue.h>

_BeginNamespace(eokas)
	
	bool llvm_jit(ast_module_t* m)
	{
		llvm::InitializeNativeTarget();
		llvm::InitializeNativeTargetAsmPrinter();
		llvm::InitializeNativeTargetAsmParser();
		
		llvm::LLVMContext context;
		
		llvm::Module* module = llvm_encode(context, m);
		if(module == nullptr)
			return false;
		
		module->print(llvm::errs(), nullptr);
		
		auto ee = llvm::EngineBuilder(std::unique_ptr<llvm::Module>(module)).setEngineKind(llvm::EngineKind::JIT).create();
		
		ee->finalizeObject();
		
		llvm::Function* func = module->getFunction("main");
		if(func == nullptr)
			return false;
		
		printf("---------------- JIT RUN ----------------\n");
		std::vector<llvm::GenericValue> args;
		auto retval = ee->runFunction(func, args);
		printf("\nRET: %s \n", retval.IntVal.toString(10, true).c_str());
		printf("---------------- JIT END ----------------\n");
		
		return true;
	}
	
	bool llvm_aot(ast_module_t* m)
	{
		llvm::InitializeAllTargetInfos();
		llvm::InitializeAllTargets();
		llvm::InitializeAllTargetMCs();
		llvm::InitializeAllAsmParsers();
		llvm::InitializeAllAsmPrinters();
		
		llvm::LLVMContext context;
		
		llvm::Module* module = llvm_encode(context, m);
		if(module == nullptr)
			return false;
		
		// DUMP CODE
		module->print(llvm::errs(), nullptr);
		
		auto targetTriple = llvm::sys::getDefaultTargetTriple();
		std::string error;
		auto target = llvm::TargetRegistry::lookupTarget(targetTriple, error);
		
		auto CPU = "generic";
		auto features = "";
		llvm::TargetOptions opt;
		auto RM = llvm::Optional<llvm::Reloc::Model>();
		auto targetMachine = target->createTargetMachine(targetTriple, CPU, features, opt, RM);
		
		module->setDataLayout(targetMachine->createDataLayout());
		module->setTargetTriple(targetTriple);
		
		auto filename = "output.o";
		std::error_code EC;
		llvm::raw_fd_ostream dest(filename, EC, llvm::sys::fs::OF_None);
		if(EC)
		{
			llvm::errs() << "Could not open file: " << EC.message();
			return false;
		}
		
		llvm::legacy::PassManager pass;
		auto fileType = llvm::CGFT_ObjectFile;
		if(targetMachine->addPassesToEmitFile(pass, dest, nullptr, fileType))
		{
			llvm::errs() << "TargetMachine can't emit a file of this type";
			return false;
		}

		pass.run(*module);
		dest.flush();
		
		return true;
	}
_EndNamespace(eokas)
