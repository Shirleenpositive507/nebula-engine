// ResourceManager is primarily template-based and defined in the header.
// This file exists only to satisfy the build system.
// Implementation details for hot-reload watching and async queue management
// are handled through the template methods defined in ResourceManager.h.
#include "ResourceManager.h"
#include "FileSystem.h"
#include <thread>
#include <chrono>
#include <atomic>

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

    static std::atomic<bool> s_reloadThreadRunning{false};

    void startBackgroundReloadThread(std::function<void()> checkFunc, int intervalMs) {
        if (s_reloadThreadRunning.exchange(true)) return;
        std::thread([checkFunc = std::move(checkFunc), intervalMs]() {
            while (s_reloadThreadRunning) {
                std::this_thread::sleep_for(std::chrono::milliseconds(intervalMs));
                if (!s_reloadThreadRunning) break;
                checkFunc();
            }
        }).detach();
    }

    void stopBackgroundReloadThread() {
        s_reloadThreadRunning = false;
    }

}
