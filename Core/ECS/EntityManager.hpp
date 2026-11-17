#pragma once
#include <pch.h>
#include <ECS/ECSType.hpp>

namespace CoreEngine
{
    namespace ECS
    {
        // Hands out entity ids and recycles the ids of destroyed entities.
        class CORE_API EntityManager
        {
        public:
            static EntityManager& GetInstance();

            EntityManager(const EntityManager&) = delete;
            EntityManager& operator=(const EntityManager&) = delete;

            // Reuses the oldest destroyed id first, otherwise makes a new one.
            EntityID Create();
            void Destroy(EntityID entityID);
            bool IsAlive(EntityID entityID) const;

            CORE_FORCE_INLINE size_t GetAliveCount() const { return mAliveCount; }

            // Forgets every entity; ids start from 0 again.
            void Clear();

        private:
            EntityManager() = default;
            ~EntityManager() = default;

        private:
            std::vector<bool> mAlive;          // Indexed by EntityID
            std::queue<EntityID> mRecycledIDs; // FIFO so a freed id isn't handed out again right away
            EntityID mNextID = 0;
            size_t mAliveCount = 0;
        };
    }
}
