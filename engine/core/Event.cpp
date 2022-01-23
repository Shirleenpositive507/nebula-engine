#include "Event.h"
#include "Logger.h"
#include <algorithm>

namespace nebula {

    void EventDispatcher::addListener(EventType type, EventCallback callback) {
        auto& callbacks = m_listeners[type];
        callbacks.push_back(std::move(callback));
    }

    void EventDispatcher::removeListener(EventType type) {
        m_listeners.erase(type);
    }

    void EventDispatcher::removeListener(EventType type, const EventCallback& callback) {
        auto it = m_listeners.find(type);
        if (it == m_listeners.end()) return;

        auto& callbacks = it->second;
        auto& target = const_cast<EventCallback&>(callback);

        callbacks.erase(
            std::remove_if(callbacks.begin(), callbacks.end(),
                [&target](const EventCallback& cb) {
                    return cb.target_type() == target.target_type();
                }),
            callbacks.end()
        );

        if (callbacks.empty()) {
            m_listeners.erase(it);
        }
    }

    void EventDispatcher::clear() {
        m_listeners.clear();
    }

    void EventDispatcher::dispatch(Event& event) {
        dispatch(event.type, event);
    }

    void EventDispatcher::dispatch(EventType type, Event& event) {
        auto it = m_listeners.find(type);
        if (it == m_listeners.end()) return;

        event.handled = false;
        event.propagating = true;

        for (auto& callback : it->second) {
            if (!event.propagating) break;
            callback(event);
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

}
