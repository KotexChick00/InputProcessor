#pragma once
#include <pch.h>
#include <ECS/ECSType.hpp>
#include <ECS/IComponent.hpp>
#include <Memory/MemoryChunkManager.hpp>
#include <Logger/Logger.hpp>
#include <iterator>
#include <new>

namespace CoreEngine
{
    namespace ECS
    {
        constexpr size_t COMPONENT_COUNT_PER_CHUNK = 64;
        constexpr size_t MAX_COMPONENT_CHUNK_COUNT = 64;
        // Stack space reserved per MemoryChunk object (the object itself is ~100 bytes on x64).
        constexpr size_t COMPONENT_CHUNK_STACK_SIZE = 256;

        // Type-erased side of a container so the ComponentManager can remove components of any type.
        class CORE_API IComponentContainer
        {
        public:
            virtual ~IComponentContainer() = default;

            virtual bool Has(EntityID entityID) const = 0;
            virtual void Remove(EntityID entityID) = 0;
            virtual void Clear() = 0;
            virtual size_t GetCount() const = 0;
        };

        // Range over every component of type T, usable with range-for.
        // Adding or removing a component of type T invalidates it.
        template <typename T>
        class ComponentView
        {
        public:
            class Iterator
            {
            public:
                using iterator_category = std::forward_iterator_tag;
                using value_type = T;
                using difference_type = std::ptrdiff_t;
                using pointer = T*;
                using reference = T&;

                Iterator() = default;
                explicit Iterator(typename std::vector<T*>::const_iterator iterator)
                    : mIterator(iterator)
                {
                }

                CORE_FORCE_INLINE T& operator*() const { return **mIterator; }
                CORE_FORCE_INLINE T* operator->() const { return *mIterator; }

                Iterator& operator++()
                {
                    ++mIterator;
                    return *this;
                }

                Iterator operator++(int)
                {
                    Iterator previous = *this;
                    ++mIterator;
                    return previous;
                }

                CORE_FORCE_INLINE bool operator==(const Iterator& other) const { return mIterator == other.mIterator; }
                CORE_FORCE_INLINE bool operator!=(const Iterator& other) const { return mIterator != other.mIterator; }

            private:
                typename std::vector<T*>::const_iterator mIterator;
            };

        public:
            ComponentView() = default;
            explicit ComponentView(const std::vector<T*>* components)
                : mComponents(components)
            {
            }

            Iterator begin() const { return mComponents ? Iterator(mComponents->begin()) : Iterator(); }
            Iterator end() const { return mComponents ? Iterator(mComponents->end()) : Iterator(); }

            CORE_FORCE_INLINE size_t Size() const { return mComponents ? mComponents->size() : 0; }
            CORE_FORCE_INLINE bool Empty() const { return Size() == 0; }

        private:
            const std::vector<T*>* mComponents = nullptr; // Borrowed
        };

        // Stores every component of type T in fixed-size blocks of a MemoryChunkManager.
        // mComponents is a packed list of the live components for fast iteration;
        // removing swaps the last component into the hole, so the order isn't stable.
        template <typename T>
        class ComponentContainer : public IComponentContainer
        {
            CORE_STATIC_ASSERT(std::is_base_of_v<IComponent, T>, "Component type must derive from IComponent!");
            CORE_STATIC_ASSERT(alignof(T) <= MAX_ALLOWED_ALIGNMENT, "Component alignment is too large for the pool allocator!");

        public:
            ComponentContainer()
                : mMemory(CreateConfiguration())
            {
            }

            ~ComponentContainer() override { Clear(); }

            ComponentContainer(const ComponentContainer&) = delete;
            ComponentContainer& operator=(const ComponentContainer&) = delete;

            template <typename... Args>
            T* Add(EntityID entityID, Args&&... args)
            {
                if (T* existing = Get(entityID))
                {
                    IP_ENGINE_WARN("Entity {} already has this component, the existing one is returned!", entityID);
                    return existing;
                }

                void* memory = mMemory.Allocate();
                if (!memory)
                {
                    IP_ENGINE_ERROR("Can't allocate memory for a component of entity {}!", entityID);
                    return nullptr;
                }

                T* component = new (memory) T(std::forward<Args>(args)...);
                static_cast<IComponent*>(component)->mOwner = entityID;

                mEntityToIndex.emplace(entityID, mComponents.size());
                mComponents.push_back(component);
                return component;
            }

            T* Get(EntityID entityID) const
            {
                auto found = mEntityToIndex.find(entityID);
                return found != mEntityToIndex.end() ? mComponents[found->second] : nullptr;
            }

            bool Has(EntityID entityID) const override { return mEntityToIndex.find(entityID) != mEntityToIndex.end(); }

            void Remove(EntityID entityID) override
            {
                auto found = mEntityToIndex.find(entityID);
                if (found == mEntityToIndex.end())
                {
                    return;
                }

                size_t index = found->second;
                T* component = mComponents[index];
                mEntityToIndex.erase(found);

                // Swap and pop to keep the list packed.
                T* last = mComponents.back();
                if (last != component)
                {
                    mComponents[index] = last;
                    mEntityToIndex[last->GetOwner()] = index;
                }
                mComponents.pop_back();

                component->~T();
                mMemory.Free(component);
            }

            void Clear() override
            {
                for (T* component : mComponents)
                {
                    component->~T();
                }
                mComponents.clear();
                mEntityToIndex.clear();
                mMemory.Clear();
            }

            size_t GetCount() const override { return mComponents.size(); }

            ComponentView<T> GetView() const { return ComponentView<T>(&mComponents); }

        private:
            static MemoryChunkManagerConfiguration CreateConfiguration()
            {
                MemoryChunkManagerConfiguration config;
                config.BlockSize = sizeof(T);
                config.BlockAlignment = static_cast<uint8_t>(alignof(T));
                config.MaxBlockCountPerChunk = COMPONENT_COUNT_PER_CHUNK;
                config.StackAllocateSize = MAX_COMPONENT_CHUNK_COUNT * COMPONENT_CHUNK_STACK_SIZE;
                return config;
            }

        private:
            MemoryChunkManager mMemory;
            std::vector<T*> mComponents;                         // Borrowed, the objects live in mMemory
            std::unordered_map<EntityID, size_t> mEntityToIndex; // EntityID -> index in mComponents
        };
    }
}
