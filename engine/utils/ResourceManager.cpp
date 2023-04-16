// ResourceManager is primarily template-based and defined in the header.
// This file exists only to satisfy the build system.
// Implementation details for hot-reload watching and async queue management
// are handled through the template methods defined in ResourceManager.h.
#include "ResourceManager.h"
#include "FileSystem.h"
#include <thread>
#include <chrono>

namespace nebula {

    static FileWatcher s_resourceWatcher;

    void startResourceWatcher(std::function<void(const std::string&)> callback) {
        s_resourceWatcher.setCallback(std::move(callback));
    }

    void addResourceWatchPath(const std::string& path) {
        s_resourceWatcher.watch(path);
    }

    void pollResourceWatcher() {
        s_resourceWatcher.poll();
    }

    void startBackgroundReloadThread(std::function<void()> checkFunc, int intervalMs) {
        std::thread([checkFunc = std::move(checkFunc), intervalMs]() {
            while (true) {
                std::this_thread::sleep_for(std::chrono::milliseconds(intervalMs));
                checkFunc();
            }
        }).detach();
    }

}
