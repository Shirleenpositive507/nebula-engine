#pragma once

#include <cstdint>
#include <cstddef>
#include <vector>
#include <mutex>
#include <atomic>

namespace nebula {

    struct PoolStatistics {
        size_t totalAllocations;
        size_t totalDeallocations;
        size_t peakUsage;
        size_t currentUsage;

        PoolStatistics() : totalAllocations(0), totalDeallocations(0), peakUsage(0), currentUsage(0) {}
    };

    enum class PoolGrowthStrategy {
        Fixed,
        Growable
    };

    class PoolAllocator {
    public:
        PoolAllocator() = default;
        ~PoolAllocator();

        PoolAllocator(const PoolAllocator&) = delete;
        PoolAllocator& operator=(const PoolAllocator&) = delete;

        PoolAllocator(PoolAllocator&& other) noexcept;
        PoolAllocator& operator=(PoolAllocator&& other) noexcept;

        void init(size_t blockSize, size_t numBlocks);
        void shutdown();

        void* allocate();
        void deallocate(void* ptr);

        void reset();

        size_t getUsedCount() const { return m_numUsedBlocks; }
        size_t getTotalCount() const { return m_numBlocks; }
        size_t getFreeCount() const { return m_numBlocks - m_numUsedBlocks; }
        size_t getMemoryUsage() const { return m_numBlocks * m_blockSize; }
        size_t getBlockSize() const { return m_blockSize; }

        bool isThreadSafe() const { return m_threadSafe; }
        void setThreadSafe(bool enabled) { m_threadSafe = enabled; }

        PoolStatistics getStatistics() const;
        void resetStatistics();

        void setGrowthStrategy(PoolGrowthStrategy strategy) { m_growthStrategy = strategy; }
        PoolGrowthStrategy getGrowthStrategy() const { return m_growthStrategy; }
        void setGrowthFactor(float factor) { m_growthFactor = factor; }
        float getGrowthFactor() const { return m_growthFactor; }

    private:
        uint8_t* m_memory = nullptr;
        size_t m_blockSize = 0;
        size_t m_numBlocks = 0;
        size_t m_numUsedBlocks = 0;

        void* m_freeList = nullptr;

        bool m_threadSafe = false;
        mutable std::mutex m_mutex;

        PoolStatistics m_statistics;
        PoolGrowthStrategy m_growthStrategy = PoolGrowthStrategy::Fixed;
        float m_growthFactor = 2.0f;

        void grow();

        struct FreeBlock {
            FreeBlock* next;
        };
    };

    class StackAllocator {
    public:
        StackAllocator() = default;
        ~StackAllocator();

        StackAllocator(const StackAllocator&) = delete;
        StackAllocator& operator=(const StackAllocator&) = delete;

        StackAllocator(StackAllocator&& other) noexcept;
        StackAllocator& operator=(StackAllocator&& other) noexcept;

        void init(size_t size);
        void shutdown();

        void* allocate(size_t size, size_t alignment = alignof(max_align_t));
        void deallocate(void* ptr);

        void reset();
        size_t getUsed() const { return m_offset; }
        size_t getTotal() const { return m_totalSize; }
        size_t getFree() const { return m_totalSize - m_offset; }

    private:
        uint8_t* m_memory = nullptr;
        size_t m_totalSize = 0;
        size_t m_offset = 0;
    };

    class FrameAllocator {
    public:
        FrameAllocator() = default;
        ~FrameAllocator();

        FrameAllocator(const FrameAllocator&) = delete;
        FrameAllocator& operator=(const FrameAllocator&) = delete;

        void init(size_t size);
        void shutdown();

        void* allocate(size_t size, size_t alignment = alignof(max_align_t));

        void beginFrame();
        void endFrame();
        void clear();

        size_t getFrameUsed() const { return m_frameOffset; }
        size_t getTotalUsed() const { return m_permanentOffset + m_frameOffset; }
        size_t getTotal() const { return m_totalSize; }

    private:
        uint8_t* m_memory = nullptr;
        size_t m_totalSize = 0;
        size_t m_permanentOffset = 0;
        size_t m_frameOffset = 0;
    };

}
