#include "Platform.h"
#include <thread>
#include <cstring>
#include <chrono>

#if defined(NEBULA_PLATFORM_LINUX) || defined(NEBULA_PLATFORM_MACOS)
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#endif

#ifdef NEBULA_PLATFORM_MACOS
#include <mach/mach.h>
#include <mach/mach_time.h>
#endif


namespace nebula {

    const char* PlatformInfo::getName() {
#ifdef NEBULA_PLATFORM_WINDOWS
        return "Windows";
#elif defined(NEBULA_PLATFORM_LINUX)
        return "Linux";
#elif defined(NEBULA_PLATFORM_MACOS)
        return "macOS";
#else
        return "Unknown";
#endif
    }

    int PlatformInfo::getCoreCount() {
#ifdef NEBULA_PLATFORM_WINDOWS
        SYSTEM_INFO sysInfo;
        GetSystemInfo(&sysInfo);
        return static_cast<int>(sysInfo.dwNumberOfProcessors);
#else
        long nprocs = sysconf(_SC_NPROCESSORS_ONLN);
        return static_cast<int>(nprocs > 0 ? nprocs : 1);
#endif
    }

    size_t PlatformInfo::getPageSize() {
#ifdef NEBULA_PLATFORM_WINDOWS
        SYSTEM_INFO sysInfo;
        GetSystemInfo(&sysInfo);
        return sysInfo.dwPageSize;
#else
        long pageSize = sysconf(_SC_PAGESIZE);
        return static_cast<size_t>(pageSize > 0 ? pageSize : 4096);
#endif
    }

    double PlatformInfo::getTime() {
        static auto startTime = std::chrono::high_resolution_clock::now();
        auto now = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::duration<double>>(now - startTime);
        return duration.count();
    }

    uint64_t PlatformInfo::getProcessId() {
#ifdef NEBULA_PLATFORM_WINDOWS
        return static_cast<uint64_t>(GetCurrentProcessId());
#else
        return static_cast<uint64_t>(getpid());
#endif
    }

    std::string PlatformInfo::getExecutablePath() {
#ifdef NEBULA_PLATFORM_WINDOWS
        char path[MAX_PATH];
        GetModuleFileNameA(nullptr, path, MAX_PATH);
        return std::string(path);
#elif defined(NEBULA_PLATFORM_LINUX)
        char path[1024];
        ssize_t len = readlink("/proc/self/exe", path, sizeof(path) - 1);
        if (len != -1) {
            path[len] = '\0';
            return std::string(path);
        }
        return "";
#elif defined(NEBULA_PLATFORM_MACOS)
        char path[1024];
        uint32_t size = sizeof(path);
        if (_NSGetExecutablePath(path, &size) == 0) {
            return std::string(path);
        }
        return "";
#endif
    }

    size_t PlatformInfo::getAvailableMemory() {
#ifdef NEBULA_PLATFORM_WINDOWS
        MEMORYSTATUSEX status;
        status.dwLength = sizeof(status);
        GlobalMemoryStatusEx(&status);
        return static_cast<size_t>(status.ullAvailPhys);
#elif defined(NEBULA_PLATFORM_LINUX)
        struct sysinfo info;
        if (sysinfo(&info) == 0) {
            return info.freeram * info.mem_unit;
        }
        return 0;
#elif defined(NEBULA_PLATFORM_MACOS)
        mach_msg_type_number_t count = HOST_VM_INFO_COUNT;
        vm_statistics_data_t vmstat;
        if (host_statistics(mach_host_self(), HOST_VM_INFO, (host_info_t)&vmstat, &count) == KERN_SUCCESS) {
            return static_cast<size_t>(vmstat.free_count) * getPageSize();
        }
        return 0;
#endif
    }

}
