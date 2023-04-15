#pragma once

#include <string>
#include <unordered_map>
#include <memory>
#include <vector>
#include <future>
#include <set>
#include <queue>
#include <functional>
#include <mutex>
#include <atomic>
#include <thread>
#include <condition_variable>

namespace nebula {

    enum class ResourcePriority {
        Low,
        Normal,
        High,
        Critical
    };

    struct AsyncLoadProgress {
        size_t total;
        size_t completed;
        size_t failed;
        bool finished;
        std::string currentTask;

        AsyncLoadProgress() : total(0), completed(0), failed(0), finished(false) {}
        float getPercent() const {
            if (total == 0) return 100.f;
            return static_cast<float>(completed + failed) / static_cast<float>(total) * 100.f;
        }
    };

    struct ResourceLoadTask {
        std::string name;
        std::string path;
        ResourcePriority priority;
        std::function<void()> onCompleted;

        ResourceLoadTask() : priority(ResourcePriority::Normal) {}
    };

    template<typename T>
    class ResourceHandle {
    public:
        ResourceHandle() = default;

        ResourceHandle(std::shared_ptr<T> resource)
            : m_resource(std::move(resource)) {}

        T* get() const { return m_resource.get(); }
        T* operator->() const { return m_resource.get(); }
        T& operator*() const { return *m_resource; }
        explicit operator bool() const { return m_resource != nullptr; }

        bool isValid() const { return m_resource != nullptr; }
        void reset() { m_resource.reset(); }

        std::shared_ptr<T> getShared() const { return m_resource; }

        int useCount() const { return m_resource.use_count(); }

    private:
        std::shared_ptr<T> m_resource;
    };

    template<typename T>
    class ResourceManager {
    public:
        using LoadFunction = std::function<std::shared_ptr<T>(const std::string& path)>;

        ResourceManager() : m_asyncRunning(false), m_threadPoolSize(2) {
            m_progress.finished = true;
        }

        virtual ~ResourceManager() {
            stopAsyncThreads();
        }

        void setLoadFunction(LoadFunction func) {
            m_loadFunc = std::move(func);
        }

        std::shared_ptr<T> load(const std::string& name, const std::string& path) {
            std::lock_guard<std::mutex> lock(m_mutex);

            auto it = m_cache.find(name);
            if (it != m_cache.end()) {
                it->second.refCount++;
                return it->second.resource;
            }

            if (!m_loadFunc) return nullptr;

            auto resource = m_loadFunc(path);
            if (resource) {
                m_cache[name] = {resource, path, {}, 1};
                m_resourceToName[resource.get()] = name;
            }
            return resource;
        }

        std::future<std::shared_ptr<T>> loadAsync(const std::string& name, const std::string& path) {
            return std::async(std::launch::async, [this, name, path]() {
                return load(name, path);
            });
        }

        std::shared_ptr<T> get(const std::string& name) {
            std::lock_guard<std::mutex> lock(m_mutex);
            auto it = m_cache.find(name);
            if (it != m_cache.end()) {
                return it->second.resource;
            }
            return nullptr;
        }

        bool has(const std::string& name) const {
            std::lock_guard<std::mutex> lock(m_mutex);
            return m_cache.find(name) != m_cache.end();
        }

        bool unload(const std::string& name) {
            std::lock_guard<std::mutex> lock(m_mutex);
            auto it = m_cache.find(name);
            if (it != m_cache.end()) {
                if (it->second.refCount > 0) {
                    it->second.refCount--;
                }
                if (it->second.refCount <= 0) {
                    m_resourceToName.erase(it->second.resource.get());
                    m_cache.erase(it);
                    return true;
                }
            }
            return false;
        }

        void unloadAll() {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_cache.clear();
            m_resourceToName.clear();
        }

        bool reload(const std::string& name) {
            std::lock_guard<std::mutex> lock(m_mutex);
            auto it = m_cache.find(name);
            if (it == m_cache.end() || !m_loadFunc) return false;

            auto newResource = m_loadFunc(it->second.path);
            if (newResource) {
                m_resourceToName.erase(it->second.resource.get());
                it->second.resource = newResource;
                m_resourceToName[newResource.get()] = name;
                return true;
            }
            return false;
        }

        void reloadAll() {
            std::vector<std::string> names;
            {
                std::lock_guard<std::mutex> lock(m_mutex);
                for (auto& [name, entry] : m_cache) {
                    names.push_back(name);
                }
            }
            for (auto& name : names) {
                reload(name);
            }
        }

        void clearUnused() {
            std::lock_guard<std::mutex> lock(m_mutex);
            for (auto it = m_cache.begin(); it != m_cache.end();) {
                if (it->second.refCount <= 0 && it->second.resource.use_count() == 1) {
                    m_resourceToName.erase(it->second.resource.get());
                    it = m_cache.erase(it);
                } else {
                    ++it;
                }
            }
        }

        std::vector<std::string> getNames() const {
            std::lock_guard<std::mutex> lock(m_mutex);
            std::vector<std::string> names;
            names.reserve(m_cache.size());
            for (auto& [name, _] : m_cache) {
                names.push_back(name);
            }
            return names;
        }

        size_t getCount() const {
            std::lock_guard<std::mutex> lock(m_mutex);
            return m_cache.size();
        }

        void setFallback(const std::string& path) {
            m_fallbackPath = path;
        }

        void addDependency(const std::string& name, const std::string& dependency) {
            std::lock_guard<std::mutex> lock(m_mutex);
            auto it = m_cache.find(name);
            if (it != m_cache.end()) {
                it->second.dependencies.push_back(dependency);
                m_dependencyGraph[name].insert(dependency);
                m_dependencyGraph[dependency];
            }
        }

        std::vector<std::string> getDependencies(const std::string& name) const {
            std::lock_guard<std::mutex> lock(m_mutex);
            auto it = m_cache.find(name);
            if (it != m_cache.end()) {
                return it->second.dependencies;
            }
            return {};
        }

        std::vector<std::string> getDependents(const std::string& name) const {
            std::lock_guard<std::mutex> lock(m_mutex);
            std::vector<std::string> result;
            for (auto& [depName, deps] : m_dependencyGraph) {
                if (deps.find(name) != deps.end()) {
                    result.push_back(depName);
                }
            }
            return result;
        }

        void reloadWithDependencies(const std::string& name) {
            std::vector<std::string> toReload;
            {
                std::lock_guard<std::mutex> lock(m_mutex);
                std::function<void(const std::string&)> collect;
                collect = [&](const std::string& n) {
                    if (std::find(toReload.begin(), toReload.end(), n) == toReload.end()) {
                        toReload.push_back(n);
                        auto depIt = m_dependencyGraph.find(n);
                        if (depIt != m_dependencyGraph.end()) {
                            for (auto& dep : depIt->second) {
                                collect(dep);
                            }
                        }
                    }
                };
                collect(name);
            }
            for (auto& n : toReload) {
                reload(n);
            }
        }

        void enqueueAsync(const std::string& name, const std::string& path,
                          ResourcePriority priority = ResourcePriority::Normal,
                          std::function<void()> onCompleted = nullptr) {
            std::lock_guard<std::mutex> lock(m_mutex);
            ResourceLoadTask task;
            task.name = name;
            task.path = path;
            task.priority = priority;
            task.onCompleted = std::move(onCompleted);
            m_asyncQueue.push(task);
            m_progress.total++;
            m_progress.finished = false;
        }

        void processAsyncQueue(int maxTasks = -1) {
            int processed = 0;
            while (maxTasks < 0 || processed < maxTasks) {
                ResourceLoadTask task;
                {
                    std::lock_guard<std::mutex> lock(m_mutex);
                    if (m_asyncQueue.empty()) {
                        m_progress.finished = true;
                        break;
                    }
                    task = std::move(m_asyncQueue.front());
                    m_asyncQueue.pop();
                }

                m_progress.currentTask = task.name;
                auto resource = load(task.name, task.path);
                if (resource) {
                    m_progress.completed++;
                } else {
                    m_progress.failed++;
                }

                if (task.onCompleted) {
                    task.onCompleted();
                }
                processed++;
            }
        }

        void startAsyncThreads(int threadCount = 2) {
            m_threadPoolSize = threadCount;
            m_asyncRunning = true;

            for (int i = 0; i < threadCount; ++i) {
                m_threads.emplace_back([this]() {
                    while (m_asyncRunning) {
                        ResourceLoadTask task;
                        {
                            std::unique_lock<std::mutex> lock(m_mutex);
                            m_cv.wait_for(lock, std::chrono::milliseconds(100), [this]() {
                                return !m_asyncQueue.empty() || !m_asyncRunning;
                            });

                            if (!m_asyncRunning) break;
                            if (m_asyncQueue.empty()) continue;

                            task = std::move(m_asyncQueue.front());
                            m_asyncQueue.pop();
                        }

                        m_progress.currentTask = task.name;
                        auto resource = load(task.name, task.path);
                        if (resource) {
                            m_progress.completed++;
                        } else {
                            m_progress.failed++;
                        }

                        if (task.onCompleted) {
                            task.onCompleted();
                        }
                    }
                });
            }
        }

        void stopAsyncThreads() {
            m_asyncRunning = false;
            m_cv.notify_all();
            for (auto& t : m_threads) {
                if (t.joinable()) t.join();
            }
            m_threads.clear();
        }

        AsyncLoadProgress getProgress() const {
            return m_progress;
        }

        void setHotReload(bool enabled) {
            m_hotReloadEnabled = enabled;
        }

        bool isHotReloadEnabled() const {
            return m_hotReloadEnabled;
        }

        void checkHotReload() {
            if (!m_hotReloadEnabled) return;

            std::vector<std::string> toReload;
            {
                std::lock_guard<std::mutex> lock(m_mutex);
                for (auto& [name, entry] : m_cache) {
                    try {
                        auto currentTime = std::filesystem::last_write_time(entry.path);
                        if (currentTime != entry.lastWriteTime) {
                            entry.lastWriteTime = currentTime;
                            toReload.push_back(name);
                        }
                    } catch (...) {}
                }
            }

            for (auto& name : toReload) {
                reloadWithDependencies(name);
            }
        }

    protected:
        struct Entry {
            std::shared_ptr<T> resource;
            std::string path;
            std::vector<std::string> dependencies;
            int refCount;
            std::filesystem::file_time_type lastWriteTime;

            Entry() : refCount(0) {}
        };

        std::unordered_map<std::string, Entry> m_cache;
        std::unordered_map<T*, std::string> m_resourceToName;
        LoadFunction m_loadFunc;
        std::string m_fallbackPath;
        mutable std::mutex m_mutex;

        std::queue<ResourceLoadTask> m_asyncQueue;
        AsyncLoadProgress m_progress;

        std::vector<std::thread> m_threads;
        std::atomic<bool> m_asyncRunning;
        int m_threadPoolSize;
        std::condition_variable m_cv;

        bool m_hotReloadEnabled;
        std::unordered_map<std::string, std::set<std::string>> m_dependencyGraph;
    };

}
