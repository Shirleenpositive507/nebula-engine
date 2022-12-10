#include "Event.h"
#include "Logger.h"
#include <algorithm>

namespace nebula {

    void EventDispatcher::addListener(EventType type, EventCallback callback, EventPriority priority) {
        m_listeners[type].push_back({std::move(callback), priority});

        auto& entries = m_listeners[type];
        std::sort(entries.begin(), entries.end(),
            [](const ListenerEntry& a, const ListenerEntry& b) {
                return static_cast<int>(a.priority) > static_cast<int>(b.priority);
            });
    }

    void EventDispatcher::removeListener(EventType type) {
        m_listeners.erase(type);
    }

    void EventDispatcher::removeListener(EventType type, const EventCallback& callback) {
        auto it = m_listeners.find(type);
        if (it == m_listeners.end()) return;

        auto& entries = it->second;
        const auto& target = callback;

        entries.erase(
            std::remove_if(entries.begin(), entries.end(),
                [&target](const ListenerEntry& entry) {
                    return entry.callback.target_type() == target.target_type();
                }),
            entries.end()
        );

        if (entries.empty()) {
            m_listeners.erase(it);
        }
    }

    void EventDispatcher::clear() {
        m_listeners.clear();
        m_filters.clear();
    }

    void EventDispatcher::dispatch(Event& event) {
        dispatch(event.type, event);
    }

    void EventDispatcher::dispatch(EventType type, Event& event) {
        auto filterIt = m_filters.find(static_cast<int>(event.getCategory()));
        if (filterIt != m_filters.end()) {
            if (!filterIt->second(event)) {
                event.filtered = true;
                return;
            }
        }

        auto it = m_listeners.find(type);
        if (it == m_listeners.end()) return;

        event.handled = false;
        event.propagating = true;

        for (auto& entry : it->second) {
            if (!event.propagating) break;
            entry.callback(event);
        }
    }

    bool EventDispatcher::hasListeners(EventType type) const {
        auto it = m_listeners.find(type);
        return it != m_listeners.end() && !it->second.empty();
    }

    int EventDispatcher::getListenerCount(EventType type) const {
        auto it = m_listeners.find(type);
        if (it == m_listeners.end()) return 0;
        return static_cast<int>(it->second.size());
    }

    void EventDispatcher::addFilter(EventCategory category, EventFilterCallback filter) {
        m_filters[static_cast<int>(category)] = std::move(filter);
    }

    void EventDispatcher::removeFilter(EventCategory category) {
        m_filters.erase(static_cast<int>(category));
    }

    void EventQueue::enqueue(EventType type, std::unique_ptr<Event> event) {
        std::lock_guard<std::mutex> lock(m_mutex);
        QueuedEvent entry;
        entry.type = type;
        entry.event = std::move(event);
        entry.priority = entry.event ? entry.event->priority : EventPriority::Normal;
        m_queue.push_back(std::move(entry));
        std::push_heap(m_queue.begin(), m_queue.end());
    }

    void EventQueue::enqueueBlocking(EventType type, std::unique_ptr<Event> event) {
        std::lock_guard<std::mutex> lock(m_mutex);
        QueuedEvent entry;
        entry.type = type;
        entry.event = std::move(event);
        entry.priority = EventPriority::Highest;
        m_blockingQueue.push_back(std::move(entry));
    }

    std::unique_ptr<Event> EventQueue::dequeue() {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (!m_blockingQueue.empty()) {
            auto entry = std::move(m_blockingQueue.front());
            m_blockingQueue.erase(m_blockingQueue.begin());
            return std::move(entry.event);
        }
        if (!m_queue.empty()) {
            std::pop_heap(m_queue.begin(), m_queue.end());
            auto entry = std::move(m_queue.back());
            m_queue.pop_back();
            return std::move(entry.event);
        }
        return nullptr;
    }

    bool EventQueue::tryDequeue(Event& event) {
        auto ptr = dequeue();
        if (ptr) {
            event = std::move(*ptr);
            return true;
        }
        return false;
    }

    size_t EventQueue::size() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_queue.size() + m_blockingQueue.size();
    }

    bool EventQueue::isEmpty() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_queue.empty() && m_blockingQueue.empty();
    }

    void EventQueue::clear() {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_queue.clear();
        m_blockingQueue.clear();
    }

    void EventQueue::processAll(EventDispatcher& dispatcher) {
        while (!isEmpty()) {
            auto event = dequeue();
            if (event) {
                dispatcher.dispatch(*event);
            }
        }
    }

    void EventBus::publish(std::unique_ptr<Event> event) {
        m_queue.enqueue(event->type, std::move(event));
    }

    void EventBus::subscribe(EventType type, EventCallback callback) {
        m_dispatcher.addListener(type, std::move(callback));
    }

    void EventBus::unsubscribe(EventType type) {
        m_dispatcher.removeListener(type);
    }

    void EventBus::process() {
        m_queue.processAll(m_dispatcher);
    }

}

