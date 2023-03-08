#pragma once
#include <vector>
#include <cstring>
#include <new>
#include <type_traits>
#include <algorithm>
#include "Component.h"

namespace nebula {

struct ComponentStorageHeader {
    u32 magic;
    u32 version;
    u32 componentTypeCount;
    u32 dataSize;
    u64 timestamp;
};

bool serializeBinary(const ComponentStorageHeader& header, const std::vector<u8>& data, const std::string& path);
bool deserializeBinary(const std::string& path, ComponentStorageHeader& header, std::vector<u8>& data);

template <typename T>
class ComponentPool : public IComponentPool {
    static_assert(std::is_base_of_v<Component, T>, "T must derive from Component");

public:
    ComponentPool(u32 capacity = 64)
        : mCapacity(capacity)
        , mComponentCount(0)
    {
        mData = static_cast<T*>(::operator new(capacity * sizeof(T)));
        mSparse.resize(capacity, UINT32_MAX);
        mDense.resize(capacity);
    }

    ~ComponentPool() override {
        clear();
        ::operator delete(mData);
    }

    ComponentPool(const ComponentPool&) = delete;
    ComponentPool& operator=(const ComponentPool&) = delete;

    ComponentPool(ComponentPool&& other) noexcept
        : mData(other.mData)
        , mSparse(std::move(other.mSparse))
        , mDense(std::move(other.mDense))
        , mCapacity(other.mCapacity)
        , mComponentCount(other.mComponentCount)
    {
        other.mData = nullptr;
        other.mComponentCount = 0;
        other.mCapacity = 0;
    }

    ComponentPool& operator=(ComponentPool&& other) noexcept {
        if (this != &other) {
            clear();
            ::operator delete(mData);
            mData = other.mData;
            mSparse = std::move(other.mSparse);
            mDense = std::move(other.mDense);
            mCapacity = other.mCapacity;
            mComponentCount = other.mComponentCount;
            other.mData = nullptr;
            other.mComponentCount = 0;
            other.mCapacity = 0;
        }
        return *this;
    }

    void add(u32 entity, const T& component) {
        if (has(entity)) {
            get(entity) = component;
            return;
        }

        if (mComponentCount >= mCapacity) {
            resize(mCapacity * 2);
        }

        u32 index = mComponentCount++;
        mDense[index] = entity;
        mSparse[entity] = index;
        new (&mData[index]) T(component);
    }

    void remove(u32 entity) {
        if (!has(entity)) return;

        u32 index = mSparse[entity];
        u32 lastIndex = mComponentCount - 1;
        u32 lastEntity = mDense[lastIndex];

        mData[index].~T();
        if (index != lastIndex) {
            new (&mData[index]) T(std::move(mData[lastIndex]));
            mData[lastIndex].~T();
            mDense[index] = lastEntity;
            mSparse[lastEntity] = index;
        }

        mSparse[entity] = UINT32_MAX;
        mComponentCount--;
    }

    T* get(u32 entity) {
        if (!has(entity)) return nullptr;
        return &mData[mSparse[entity]];
    }

    void* get(void* entity) override {
        return get(static_cast<u32>(reinterpret_cast<uintptr_t>(entity)));
    }

    bool has(u32 entity) const {
        return entity < mSparse.size() && mSparse[entity] != UINT32_MAX;
    }

    void clear() {
        for (u32 i = 0; i < mComponentCount; ++i) {
            mData[i].~T();
        }
        mComponentCount = 0;
        std::fill(mSparse.begin(), mSparse.end(), UINT32_MAX);
    }

    T* getDataArray() {
        return mData;
    }

    const T* getDataArray() const {
        return mData;
    }

    void resize(u32 newCapacity) {
        if (newCapacity <= mCapacity) return;

        T* newData = static_cast<T*>(::operator new(newCapacity * sizeof(T)));
        for (u32 i = 0; i < mComponentCount; ++i) {
            new (&newData[i]) T(std::move(mData[i]));
            mData[i].~T();
        }
        ::operator delete(mData);
        mData = newData;

        mSparse.resize(newCapacity, UINT32_MAX);
        mDense.resize(newCapacity);
        mCapacity = newCapacity;
    }

    u32 getCapacity() const { return mCapacity; }
    u32 getComponentCount() const { return mComponentCount; }
    const std::vector<u32>& getDenseArray() const { return mDense; }

    void create(void* entity) override {
        u32 e = static_cast<u32>(reinterpret_cast<uintptr_t>(entity));
        if (!has(e)) {
            T comp;
            add(e, comp);
        }
    }

    bool serialize(const std::string& filepath) const {
        FILE* file = nullptr;
        fopen_s(&file, filepath.c_str(), "wb");
        if (!file) return false;

        fwrite(&mComponentCount, sizeof(mComponentCount), 1, file);
        for (u32 i = 0; i < mComponentCount; ++i) {
            fwrite(&mDense[i], sizeof(u32), 1, file);
            fwrite(&mData[i], sizeof(T), 1, file);
        }
        fclose(file);
        return true;
    }

    bool deserialize(const std::string& filepath) {
        FILE* file = nullptr;
        fopen_s(&file, filepath.c_str(), "rb");
        if (!file) return false;

        clear();
        u32 count;
        fread(&count, sizeof(count), 1, file);
        for (u32 i = 0; i < count; ++i) {
            u32 entityId;
            fread(&entityId, sizeof(entityId), 1, file);
            T comp;
            fread(&comp, sizeof(T), 1, file);
            add(entityId, comp);
        }
        fclose(file);
        return true;
    }

    void recycleRemovedSlot(u32 entity) {
        if (mSparse[entity] != UINT32_MAX) {
            remove(entity);
        }
    }

    u32 getRemovedCount() const {
        u32 removed = 0;
        for (u32 i = 0; i < mCapacity; ++i) {
            if (mSparse[i] == UINT32_MAX) removed++;
        }
        return removed;
    }

    class Iterator {
    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using pointer = T*;
        using reference = T&;

        Iterator(T* data, u32* dense, u32 index) : mData(data), mDense(dense), mIndex(index) {}

        reference operator*() const { return mData[mIndex]; }
        pointer operator->() { return &mData[mIndex]; }
        Iterator& operator++() { ++mIndex; return *this; }
        Iterator operator++(int) { Iterator tmp = *this; ++(*this); return tmp; }
        bool operator==(const Iterator& other) const { return mIndex == other.mIndex; }
        bool operator!=(const Iterator& other) const { return mIndex != other.mIndex; }

    private:
        T* mData;
        u32* mDense;
        u32 mIndex;
    };

    Iterator begin() { return Iterator(mData, mDense.data(), 0); }
    Iterator end() { return Iterator(mData, mDense.data(), mComponentCount); }

    void copyComponent(u32 srcEntity, u32 dstEntity, ComponentPool<T>& dstPool) {
        if (!has(srcEntity)) return;
        T& srcComp = mData[mSparse[srcEntity]];
        dstPool.add(dstEntity, srcComp);
    }

private:
    T* mData;
    std::vector<u32> mSparse;
    std::vector<u32> mDense;
    u32 mCapacity;
    u32 mComponentCount;
};

}
