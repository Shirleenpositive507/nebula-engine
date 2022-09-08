#pragma once

#include <functional>
#include <vector>
#include <memory>
#include <algorithm>

namespace nebula {

    template<typename T>
    class Delegate;

    template<typename Ret, typename... Args>
    class Delegate<Ret(Args...)> {
    public:
        using FunctionType = std::function<Ret(Args...)>;

        Delegate() = default;

        template<typename F>
        void bind(F&& func) {
            m_function = std::forward<F>(func);
        }

        template<typename Class>
        void bind(Class* instance, Ret(Class::*method)(Args...)) {
            m_function = [instance, method](Args... args) -> Ret {
                return (instance->*method)(std::forward<Args>(args)...);
            };
        }

        template<typename Class>
        void bindConst(const Class* instance, Ret(Class::*method)(Args...) const) {
            m_function = [instance, method](Args... args) -> Ret {
                return (instance->*method)(std::forward<Args>(args)...);
            };
        }

        void unbind() {
            m_function = nullptr;
        }

        bool isBound() const {
            return static_cast<bool>(m_function);
        }

        Ret invoke(Args... args) const {
            return m_function(std::forward<Args>(args)...);
        }

        Ret invokeIfBound(Args... args) const {
            if (m_function) {
                return m_function(std::forward<Args>(args)...);
            }
            if constexpr (!std::is_same_v<Ret, void>) {
                return Ret();
            }
        }

        Ret operator()(Args... args) const {
            return invoke(std::forward<Args>(args)...);
        }

        void clear() {
            m_function = nullptr;
        }

    private:
        FunctionType m_function;
    };

    struct Connection {
        size_t id = 0;
        bool valid = false;

        Connection() = default;
        Connection(size_t id) : id(id), valid(true) {}

        void disconnect() { valid = false; }
        bool isConnected() const { return valid; }
    };

    template<typename T>
    class MulticastDelegate;

    template<typename... Args>
    class MulticastDelegate<void(Args...)> {
    public:
        using FunctionType = std::function<void(Args...)>;

        Connection bind(FunctionType func) {
            size_t id = m_nextId++;
            m_entries.push_back({id, std::move(func)});
            return Connection(id);
        }

        template<typename Class>
        Connection bind(Class* instance, void(Class::*method)(Args...)) {
            return bind([instance, method](Args... args) {
                (instance->*method)(std::forward<Args>(args)...);
            });
        }

        bool unbind(const Connection& connection) {
            auto it = std::remove_if(m_entries.begin(), m_entries.end(),
                [&](const Entry& e) { return e.id == connection.id; });
            if (it != m_entries.end()) {
                m_entries.erase(it, m_entries.end());
                return true;
            }
            return false;
        }

        void unbindAll() {
            m_entries.clear();
        }

        void invoke(Args... args) const {
            for (auto& entry : m_entries) {
                entry.function(std::forward<Args>(args)...);
            }
        }

        void operator()(Args... args) const {
            invoke(std::forward<Args>(args)...);
        }

        size_t getCount() const { return m_entries.size(); }
        bool isEmpty() const { return m_entries.empty(); }
        void clear() { m_entries.clear(); }

    private:
        struct Entry {
            size_t id;
            FunctionType function;
        };

        std::vector<Entry> m_entries;
        size_t m_nextId = 1;
    };

    template<typename... Args>
    using Event = MulticastDelegate<void(Args...)>;

    class ScopedConnection {
    public:
        ScopedConnection() = default;

        ScopedConnection(const Connection& connection)
            : m_connection(std::make_shared<Connection>(connection)) {}

        ScopedConnection(const ScopedConnection& other)
            : m_connection(other.m_connection) {}

        ScopedConnection(ScopedConnection&& other) noexcept
            : m_connection(std::move(other.m_connection)) {}

        ScopedConnection& operator=(const ScopedConnection& other) {
            if (this != &other) {
                disconnect();
                m_connection = other.m_connection;
            }
            return *this;
        }

        ScopedConnection& operator=(ScopedConnection&& other) noexcept {
            if (this != &other) {
                disconnect();
                m_connection = std::move(other.m_connection);
            }
            return *this;
        }

        ~ScopedConnection() {
            disconnect();
        }

        void disconnect() {
            if (m_connection) {
                m_connection->disconnect();
            }
        }

        bool isConnected() const {
            return m_connection && m_connection->isConnected();
        }

        Connection* get() const { return m_connection.get(); }

    private:
        std::shared_ptr<Connection> m_connection;
    };

}
