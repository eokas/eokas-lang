cmake_minimum_required(VERSION 3.22)

cmake_host_system_information(RESULT OS_NAME QUERY OS_NAME)
message("OS_NNAME = " ${OS_NAME})

project(eokas)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)

include_directories(
        deps/eokas-base/include
        $ENV{LLVM_SDK_PATH}/include
)
link_directories(
        deps/eokas-base/lib/${OS_NAME}/${CMAKE_BUILD_TYPE}
        $ENV{LLVM_SDK_PATH}/lib
)

message("CMAKE_BUILD_TYPE: " ${CMAKE_BUILD_TYPE})
message("LLVM_SDK_PATH: " $ENV{LLVM_SDK_PATH})

set(LLVM_DEPS m z curses xml2)
set(LLVM_LIBS LLVMWindowsManifest LLVMXRay LLVMLibDriver LLVMDlltoolDriver LLVMCoverage LLVMLineEditor LLVMXCoreDisassembler LLVMXCoreCodeGen LLVMXCoreDesc LLVMXCoreInfo LLVMX86Disassembler LLVMX86AsmParser LLVMX86CodeGen LLVMX86Desc LLVMX86Info LLVMWebAssemblyDisassembler LLVMWebAssemblyAsmParser LLVMWebAssemblyCodeGen LLVMWebAssemblyDesc LLVMWebAssemblyInfo LLVMSystemZDisassembler LLVMSystemZAsmParser LLVMSystemZCodeGen LLVMSystemZDesc LLVMSystemZInfo LLVMSparcDisassembler LLVMSparcAsmParser LLVMSparcCodeGen LLVMSparcDesc LLVMSparcInfo LLVMRISCVDisassembler LLVMRISCVAsmParser LLVMRISCVCodeGen LLVMRISCVDesc LLVMRISCVInfo LLVMPowerPCDisassembler LLVMPowerPCAsmParser LLVMPowerPCCodeGen LLVMPowerPCDesc LLVMPowerPCInfo LLVMNVPTXCodeGen LLVMNVPTXDesc LLVMNVPTXInfo LLVMMSP430Disassembler LLVMMSP430AsmParser LLVMMSP430CodeGen LLVMMSP430Desc LLVMMSP430Info LLVMMipsDisassembler LLVMMipsAsmParser LLVMMipsCodeGen LLVMMipsDesc LLVMMipsInfo LLVMLanaiDisassembler LLVMLanaiCodeGen LLVMLanaiAsmParser LLVMLanaiDesc LLVMLanaiInfo LLVMHexagonDisassembler LLVMHexagonCodeGen LLVMHexagonAsmParser LLVMHexagonDesc LLVMHexagonInfo LLVMBPFDisassembler LLVMBPFAsmParser LLVMBPFCodeGen LLVMBPFDesc LLVMBPFInfo LLVMAVRDisassembler LLVMAVRAsmParser LLVMAVRCodeGen LLVMAVRDesc LLVMAVRInfo LLVMARMDisassembler LLVMARMAsmParser LLVMARMCodeGen LLVMARMDesc LLVMARMUtils LLVMARMInfo LLVMAMDGPUDisassembler LLVMAMDGPUAsmParser LLVMAMDGPUCodeGen LLVMAMDGPUDesc LLVMAMDGPUUtils LLVMAMDGPUInfo LLVMAArch64Disassembler LLVMAArch64AsmParser LLVMAArch64CodeGen LLVMAArch64Desc LLVMAArch64Utils LLVMAArch64Info LLVMOrcJIT LLVMMCJIT LLVMJITLink LLVMOrcTargetProcess LLVMOrcShared LLVMInterpreter LLVMExecutionEngine LLVMRuntimeDyld LLVMSymbolize LLVMDebugInfoPDB LLVMDebugInfoGSYM LLVMOption LLVMObjectYAML LLVMMCA LLVMMCDisassembler LLVMLTO LLVMCFGuard LLVMFrontendOpenACC LLVMExtensions Polly PollyISL LLVMPasses LLVMObjCARCOpts LLVMHelloNew LLVMCoroutines LLVMipo LLVMInstrumentation LLVMVectorize LLVMLinker LLVMFrontendOpenMP LLVMDWARFLinker LLVMGlobalISel LLVMMIRParser LLVMAsmPrinter LLVMDebugInfoDWARF LLVMSelectionDAG LLVMCodeGen LLVMIRReader LLVMAsmParser LLVMInterfaceStub LLVMFileCheck LLVMFuzzMutate LLVMTarget LLVMScalarOpts LLVMInstCombine LLVMAggressiveInstCombine LLVMTransformUtils LLVMBitWriter LLVMAnalysis LLVMProfileData LLVMObject LLVMTextAPI LLVMMCParser LLVMMC LLVMDebugInfoCodeView LLVMDebugInfoMSF LLVMBitReader LLVMCore LLVMRemarks LLVMBitstreamReader LLVMBinaryFormat LLVMTableGen LLVMSupport LLVMDemangle)
set(LIBS eokas-base ${LLVM_DEPS} ${LLVM_LIBS})

aux_source_directory(src SRC)
aux_source_directory(src/app SRC_PARSER)
aux_source_directory(src/ast SRC_AST)
aux_source_directory(src/llvm SRC_LLVM)
aux_source_directory(src/omis SRC_OMIS)

add_executable(${PROJECT_NAME} ${SRC} ${SRC_PARSER} ${SRC_AST} ${SRC_LLVM} ${SRC_OMIS}
        src/llvm/bridge.cpp
        src/llvm/bridge.h)
target_link_libraries(${PROJECT_NAME} ${LIBS})
