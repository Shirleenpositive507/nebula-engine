#include "TaskSystem.h"
#include <algorithm>

namespace nebula {

TaskSystem::TaskSystem(size_t numThreads)
    : m_running(false)
    , m_numThreads(numThreads == 0 ? std::thread::hardware_concurrency() : numThreads)
{
    if (m_numThreads == 0) m_numThreads = 2;
}

TaskSystem::~TaskSystem() {
    stop();
}

void TaskSystem::start() {
    if (m_running) return;
    m_running = true;
    for (size_t i = 0; i < m_numThreads; ++i) {
        m_threads.emplace_back(&TaskSystem::workerLoop, this);
    }
}

void TaskSystem::stop() {
    if (!m_running) return;
    m_running = false;
    m_cv.notify_all();
    for (auto& t : m_threads) {
        if (t.joinable()) t.join();
    }
    m_threads.clear();
}

void TaskSystem::setThreadCount(size_t count) {
    if (m_running) {
        stop();
        m_numThreads = count;
        start();
    } else {
        m_numThreads = count;
    }
}

void TaskSystem::workerLoop() {
    while (m_running) {
        Task task = dequeueTask();
        if (task.func) {
            task.func();
        }
    }
}

TaskSystem::Task TaskSystem::dequeueTask() {
    std::unique_lock<std::mutex> lock(m_mutex);
    m_cv.wait(lock, [this]() { return !m_queue.empty() || !m_running; });
    if (!m_running && m_queue.empty()) {
        return Task{ nullptr, TaskPriority::Normal, TaskGroup() };
    }
    Task task = m_queue.top();
    m_queue.pop();
    return task;
}

void TaskSystem::waitForAll() {
    while (true) {
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            if (m_queue.empty()) break;
        }
        std::this_thread::yield();
    }
}

size_t TaskSystem::pendingTasks() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_queue.size();
}

TaskSystem::TaskGroup TaskSystem::createTaskGroup() {
    return TaskGroup();
}

void TaskSystem::waitForGroup(const TaskGroup& group) {
    while (group.counter->load(std::memory_order_relaxed) > 0) {
        Task task = dequeueTask();
        if (task.func) {
            task.func();
        }
    }
}

}

