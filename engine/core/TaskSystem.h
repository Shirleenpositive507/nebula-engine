#pragma once

#include <functional>
#include <future>
#include <memory>
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>

namespace nebula {

enum class TaskPriority {
    Low = 0,
    Normal,
    High,
    Critical
};

class TaskSystem {
public:
    explicit TaskSystem(size_t numThreads = 0);
    ~TaskSystem();

    void start();
    void stop();
    bool isRunning() const { return m_running; }

    size_t getThreadCount() const { return m_threads.size(); }
    void setThreadCount(size_t count);

    template<typename F, typename... Args>
    auto submit(TaskPriority priority, F&& f, Args&&... args)
        -> std::future<decltype(f(args...))>;

    template<typename F, typename... Args>
    auto submit(F&& f, Args&&... args)
        -> std::future<decltype(f(args...))>;

    void waitForAll();
    size_t pendingTasks() const;

    template<typename Iter, typename F>
    void parallelFor(Iter begin, Iter end, F&& func);

    template<typename Iter, typename F>
    void parallelFor(Iter begin, Iter end, F&& func, size_t chunkSize);

    template<typename... Fs>
    void parallelInvoke(Fs&&... funcs);

    struct TaskGroup {
        std::shared_ptr<std::atomic<int>> counter;
        TaskGroup() : counter(std::make_shared<std::atomic<int>>(0)) {}
    };

    TaskGroup createTaskGroup();
    void waitForGroup(const TaskGroup& group);

private:
    struct Task {
        std::function<void()> func;
        TaskPriority priority;
        TaskGroup group;

        bool operator<(const Task& other) const {
            return priority < other.priority;
        }
    };

    void workerLoop();
    Task dequeueTask();

    std::vector<std::thread> m_threads;
    std::priority_queue<Task> m_queue;
    mutable std::mutex m_mutex;
    std::condition_variable m_cv;
    std::atomic<bool> m_running;
    size_t m_numThreads;
};

template<typename F, typename... Args>
auto TaskSystem::submit(TaskPriority priority, F&& f, Args&&... args)
    -> std::future<decltype(f(args...))>
{
    using return_type = decltype(f(args...));
    auto task = std::make_shared<std::packaged_task<return_type()>>(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...)
    );
    std::future<return_type> result = task->get_future();

    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_queue.push(Task{ [task]() { (*task)(); }, priority, TaskGroup() });
    }
    m_cv.notify_one();
    return result;
}

template<typename F, typename... Args>
auto TaskSystem::submit(F&& f, Args&&... args)
    -> std::future<decltype(f(args...))>
{
    return submit(TaskPriority::Normal, std::forward<F>(f), std::forward<Args>(args)...);
}

template<typename Iter, typename F>
void TaskSystem::parallelFor(Iter begin, Iter end, F&& func) {
    size_t count = static_cast<size_t>(std::distance(begin, end));
    size_t numThreads = std::max(size_t(1), m_threads.size());
    size_t chunkSize = (count + numThreads - 1) / numThreads;
    parallelFor(begin, end, std::forward<F>(func), chunkSize);
}

template<typename Iter, typename F>
void TaskSystem::parallelFor(Iter begin, Iter end, F&& func, size_t chunkSize) {
    size_t count = static_cast<size_t>(std::distance(begin, end));
    if (count == 0) return;

    TaskGroup group = createTaskGroup();

    Iter current = begin;
    while (current != end) {
        Iter chunkEnd = current;
        size_t remaining = static_cast<size_t>(std::distance(current, end));
        size_t step = std::min(chunkSize, remaining);
        std::advance(chunkEnd, static_cast<ptrdiff_t>(step));

        group.counter->fetch_add(1, std::memory_order_relaxed);

        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_queue.push(Task{
                [current, chunkEnd, func = std::forward<F>(func), group]() mutable {
                    for (Iter it = current; it != chunkEnd; ++it) {
                        func(*it);
                    }
                    group.counter->fetch_sub(1, std::memory_order_relaxed);
                },
                TaskPriority::Normal,
                group
            });
        }
        m_cv.notify_one();

        current = chunkEnd;
    }
}

template<typename... Fs>
void TaskSystem::parallelInvoke(Fs&&... funcs) {
    TaskGroup group = createTaskGroup();
    (([this, &group, func = std::forward<Fs>(funcs)]() mutable {
        group.counter->fetch_add(1, std::memory_order_relaxed);
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_queue.push(Task{
                [func = std::move(func), group]() mutable {
                    func();
                    group.counter->fetch_sub(1, std::memory_order_relaxed);
                },
                TaskPriority::Normal,
                group
            });
        }
        m_cv.notify_one();
    })(), ...);
}

}

