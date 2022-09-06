#include "PoolAllocator.h"
#include <cstdlib>
#include <cstring>
#include <algorithm>

#ifdef NEBULA_PLATFORM_WINDOWS
#include <malloc.h>
#define NEBULA_ALIGNED_ALLOC(size, align) _aligned_malloc(size, align)
#define NEBULA_ALIGNED_FREE(ptr) _aligned_free(ptr)
#else
#define NEBULA_ALIGNED_ALLOC(size, align) std::aligned_alloc(align, size)
#define NEBULA_ALIGNED_FREE(ptr) std::free(ptr)
#endif

namespace nebula {

    PoolAllocator::PoolAllocator(PoolAllocator&& other) noexcept
        : m_memory(other.m_memory), m_blockSize(other.m_blockSize),
          m_numBlocks(other.m_numBlocks), m_numUsedBlocks(other.m_numUsedBlocks),
          m_freeList(other.m_freeList)
    {
        other.m_memory = nullptr;
        other.m_freeList = nullptr;
        other.m_numBlocks = 0;
        other.m_numUsedBlocks = 0;
    }

    PoolAllocator& PoolAllocator::operator=(PoolAllocator&& other) noexcept {
        if (this != &other) {
            shutdown();
            m_memory = other.m_memory;
            m_blockSize = other.m_blockSize;
            m_numBlocks = other.m_numBlocks;
            m_numUsedBlocks = other.m_numUsedBlocks;
            m_freeList = other.m_freeList;
            other.m_memory = nullptr;
            other.m_freeList = nullptr;
            other.m_numBlocks = 0;
            other.m_numUsedBlocks = 0;
        }
        return *this;
    }

    PoolAllocator::~PoolAllocator() {
        shutdown();
    }

    void PoolAllocator::init(size_t blockSize, size_t numBlocks) {
        shutdown();

        if (blockSize < sizeof(FreeBlock)) {
            blockSize = sizeof(FreeBlock);
        }

        m_blockSize = blockSize;
        m_numBlocks = numBlocks;
        m_numUsedBlocks = 0;

        m_memory = static_cast<uint8_t*>(NEBULA_ALIGNED_ALLOC(blockSize * numBlocks, alignof(max_align_t)));
        if (!m_memory) return;

        m_freeList = m_memory;
        FreeBlock* current = static_cast<FreeBlock*>(m_freeList);
        for (size_t i = 0; i < numBlocks - 1; ++i) {
            current->next = reinterpret_cast<FreeBlock*>(m_memory + (i + 1) * blockSize);
            current = current->next;
        }
        current->next = nullptr;
    }

    void PoolAllocator::shutdown() {
        if (m_memory) {
            NEBULA_ALIGNED_FREE(m_memory);
            m_memory = nullptr;
        }
        m_freeList = nullptr;
        m_numBlocks = 0;
        m_numUsedBlocks = 0;
    }

    void* PoolAllocator::allocate() {
        if (m_threadSafe) {
            std::lock_guard<std::mutex> lock(m_mutex);
            if (!m_freeList) return nullptr;
            FreeBlock* block = static_cast<FreeBlock*>(m_freeList);
            m_freeList = block->next;
            ++m_numUsedBlocks;
            std::memset(block, 0, m_blockSize);
            return block;
        }

        if (!m_freeList) return nullptr;
        FreeBlock* block = static_cast<FreeBlock*>(m_freeList);
        m_freeList = block->next;
        ++m_numUsedBlocks;
        std::memset(block, 0, m_blockSize);
        return block;
    }

    void PoolAllocator::deallocate(void* ptr) {
        if (!ptr) return;

        if (m_threadSafe) {
            std::lock_guard<std::mutex> lock(m_mutex);
            FreeBlock* block = static_cast<FreeBlock*>(ptr);
            block->next = static_cast<FreeBlock*>(m_freeList);
            m_freeList = block;
            --m_numUsedBlocks;
            return;
        }

        FreeBlock* block = static_cast<FreeBlock*>(ptr);
        block->next = static_cast<FreeBlock*>(m_freeList);
        m_freeList = block;
        --m_numUsedBlocks;
    }

    void PoolAllocator::reset() {
        if (m_threadSafe) {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_freeList = m_memory;
            FreeBlock* current = static_cast<FreeBlock*>(m_freeList);
            for (size_t i = 0; i < m_numBlocks - 1; ++i) {
                current->next = reinterpret_cast<FreeBlock*>(m_memory + (i + 1) * m_blockSize);
                current = current->next;
            }
            current->next = nullptr;
            m_numUsedBlocks = 0;
            return;
        }

        m_freeList = m_memory;
        FreeBlock* current = static_cast<FreeBlock*>(m_freeList);
        for (size_t i = 0; i < m_numBlocks - 1; ++i) {
            current->next = reinterpret_cast<FreeBlock*>(m_memory + (i + 1) * m_blockSize);
            current = current->next;
        }
        current->next = nullptr;
        m_numUsedBlocks = 0;
    }

    StackAllocator::StackAllocator(StackAllocator&& other) noexcept
        : m_memory(other.m_memory), m_totalSize(other.m_totalSize), m_offset(other.m_offset)
    {
        other.m_memory = nullptr;
        other.m_totalSize = 0;
        other.m_offset = 0;
    }

    StackAllocator& StackAllocator::operator=(StackAllocator&& other) noexcept {
        if (this != &other) {
            shutdown();
            m_memory = other.m_memory;
            m_totalSize = other.m_totalSize;
            m_offset = other.m_offset;
            other.m_memory = nullptr;
            other.m_totalSize = 0;
            other.m_offset = 0;
        }
        return *this;
    }

    StackAllocator::~StackAllocator() {
        shutdown();
    }

    void StackAllocator::init(size_t size) {
        shutdown();
        m_totalSize = size;
        m_offset = 0;
        m_memory = static_cast<uint8_t*>(NEBULA_ALIGNED_ALLOC(size, alignof(max_align_t)));
    }

    void StackAllocator::shutdown() {
        if (m_memory) {
            NEBULA_ALIGNED_FREE(m_memory);
            m_memory = nullptr;
        }
        m_totalSize = 0;
        m_offset = 0;
    }

    void* StackAllocator::allocate(size_t size, size_t alignment) {
        uintptr_t current = reinterpret_cast<uintptr_t>(m_memory + m_offset);
        uintptr_t aligned = (current + alignment - 1) & ~(alignment - 1);
        size_t alignedOffset = m_offset + (aligned - current);

        if (alignedOffset + size > m_totalSize) return nullptr;

        m_offset = alignedOffset + size;
        return m_memory + alignedOffset;
    }

    void StackAllocator::deallocate(void* ptr) {
        (void)ptr;
    }

    void StackAllocator::reset() {
        m_offset = 0;
    }

    FrameAllocator::~FrameAllocator() {
        shutdown();
    }

    void FrameAllocator::init(size_t size) {
        m_totalSize = size;
        m_permanentOffset = 0;
        m_frameOffset = 0;
        m_memory = static_cast<uint8_t*>(NEBULA_ALIGNED_ALLOC(size, alignof(max_align_t)));
    }

    void FrameAllocator::shutdown() {
        if (m_memory) {
            NEBULA_ALIGNED_FREE(m_memory);
            m_memory = nullptr;
        }
        m_totalSize = 0;
        m_permanentOffset = 0;
        m_frameOffset = 0;
    }

    void* FrameAllocator::allocate(size_t size, size_t alignment) {
        size_t currentOffset = m_permanentOffset + m_frameOffset;
        uintptr_t current = reinterpret_cast<uintptr_t>(m_memory + currentOffset);
        uintptr_t aligned = (current + alignment - 1) & ~(alignment - 1);
        size_t alignedOffset = currentOffset + (aligned - current);

        if (alignedOffset + size > m_totalSize) return nullptr;

        m_frameOffset = alignedOffset - m_permanentOffset + size;
        return m_memory + alignedOffset;
    }

    void FrameAllocator::beginFrame() {
        m_frameOffset = 0;
    }

    void FrameAllocator::endFrame() {
        m_frameOffset = 0;
    }

    void FrameAllocator::clear() {
        m_permanentOffset = 0;
        m_frameOffset = 0;
    }

}
