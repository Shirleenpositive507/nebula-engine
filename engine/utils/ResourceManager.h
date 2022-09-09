#pragma once

#include <string>
#include <unordered_map>
#include <memory>
#include <vector>
#include <future>
#include <set>
#include <functional>
#include <mutex>

namespace nebula {

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

    private:
        std::shared_ptr<T> m_resource;
    };

    template<typename T>
    class ResourceManager {
    public:
        using LoadFunction = std::function<std::shared_ptr<T>(const std::string& path)>;

        ResourceManager() = default;
        virtual ~ResourceManager() = default;

        void setLoadFunction(LoadFunction func) {
            m_loadFunc = std::move(func);
        }

        std::shared_ptr<T> load(const std::string& name, const std::string& path) {
            std::lock_guard<std::mutex> lock(m_mutex);

            auto it = m_cache.find(name);
            if (it != m_cache.end()) {
                return it->second.resource;
            }

            if (!m_loadFunc) return nullptr;

            auto resource = m_loadFunc(path);
            if (resource) {
                m_cache[name] = {resource, path, {}};
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
                m_resourceToName.erase(it->second.resource.get());
                m_cache.erase(it);
                return true;
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
                if (it->second.resource.use_count() == 1) {
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

    protected:
        struct Entry {
            std::shared_ptr<T> resource;
            std::string path;
            std::vector<std::string> dependencies;
        };

        std::unordered_map<std::string, Entry> m_cache;
        std::unordered_map<T*, std::string> m_resourceToName;
        LoadFunction m_loadFunc;
        std::string m_fallbackPath;
        mutable std::mutex m_mutex;
    };

}
