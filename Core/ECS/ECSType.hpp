#pragma once
#include <pch.h>

namespace CoreEngine
{
    namespace ECS
    {
        // Lower value runs first when two systems have no dependency between them.
        enum class ESystemPriority
        {
            Low = 300,
            Medium = 200,
            High = 100
        };

        typedef size_t EntityID;
        typedef size_t ComponentID;
        typedef size_t ComponentTypeID;
        typedef size_t SystemID;
        typedef size_t SystemTypeID;

        constexpr EntityID INVALID_ENTITY_ID = std::numeric_limits<EntityID>::max();

        // Hands out consecutive ids (0, 1, 2, ...) per family, one per type T,
        // so the ids can index a vector.
        template <typename TFamily>
        class TypeIDGenerator
        {
        public:
            template <typename T>
            static size_t Get()
            {
                static const size_t sTypeID = sNextTypeID++;
                return sTypeID;
            }

        private:
            static inline size_t sNextTypeID = 0;
        };
    }
}
