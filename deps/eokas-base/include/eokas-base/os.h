
#ifndef _EOKAS_BASE_OS_H_
#define _EOKAS_BASE_OS_H_

#include "./header.h"
#include "./String.h"

namespace eokas
{
    struct CpuState
    {
        u64_t frequency = 0;
    };

    struct MemoryState
    {
        size_t total = 0;
        size_t available = 0;
    };

    struct OS
    {
        static String getSystemName();
        static String getSystemVersion();

        static String getDeviceName();

        static u32_t getCpuCount();
        static CpuState& getCpuState(u32_t index);

        static MemoryState& getMemoryState();

        static String getEnv(const String& name);
        static void setEnv(const String& name, const String& value);

        static String getCurrentUser();
        static u32_t getCurrentProcess();
        static u32_t getCurrentThread();
    };
}

#endif //_EOKAS_BASE_OS_H_
