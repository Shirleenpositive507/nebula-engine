#pragma once

#ifdef _WIN32
    #define NEBULA_PLATFORM_WINDOWS
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif
    #ifndef NOMINMAX
        #define NOMINMAX
    #endif
    #include <windows.h>
    #include <intrin.h>
#elif defined(__linux__)
    #define NEBULA_PLATFORM_LINUX
    #include <unistd.h>
    #include <sys/sysinfo.h>
#elif defined(__APPLE__)
    #define NEBULA_PLATFORM_MACOS
    #include <unistd.h>
    #include <sys/sysctl.h>
#else
    #error "Unsupported platform"
#endif

#include <cstdint>
#include <cstddef>
#include <string>

namespace nebula {

#ifdef NEBULA_PLATFORM_WINDOWS
    using ThreadHandle = HANDLE;
    using LibraryHandle = HMODULE;
    using ProcessId = DWORD;
#else
    using ThreadHandle = pthread_t;
    using LibraryHandle = void*;
    using ProcessId = pid_t;
#endif

    struct PlatformInfo {
        static const char* getName();
        static int getCoreCount();
        static size_t getPageSize();
        static double getTime();
        static uint64_t getProcessId();
        static std::string getExecutablePath();
        static size_t getAvailableMemory();
    };

    inline void platformSleep(double seconds) {
#ifdef NEBULA_PLATFORM_WINDOWS
        Sleep(static_cast<DWORD>(seconds * 1000.0));
#else
        usleep(static_cast<useconds_t>(seconds * 1000000.0));
#endif
    }

    inline uint64_t getHardwareConcurrency() {
#ifdef NEBULA_PLATFORM_WINDOWS
        SYSTEM_INFO sysInfo;
        GetSystemInfo(&sysInfo);
        return sysInfo.dwNumberOfProcessors;
#else
        return sysconf(_SC_NPROCESSORS_ONLN);
#endif
    }

    inline void setThreadName(const char* name) {
#ifdef NEBULA_PLATFORM_WINDOWS
        const DWORD MS_VC_EXCEPTION = 0x406D1388;
#pragma pack(push, 8)
        struct THREADNAME_INFO {
            DWORD dwType;
            LPCSTR szName;
            DWORD dwThreadID;
            DWORD dwFlags;
        };
#pragma pack(pop)
        THREADNAME_INFO info;
        info.dwType = 0x1000;
        info.szName = name;
        info.dwThreadID = GetCurrentThreadId();
        info.dwFlags = 0;
        __try {
            RaiseException(MS_VC_EXCEPTION, 0, sizeof(info) / sizeof(ULONG_PTR), (ULONG_PTR*)&info);
        } __except (EXCEPTION_EXECUTE_HANDLER) {}
#elif defined(NEBULA_PLATFORM_LINUX)
        pthread_setname_np(pthread_self(), name);
#elif defined(NEBULA_PLATFORM_MACOS)
        pthread_setname_np(name);
#endif
    }

}
