#pragma once
#include <pch.h>
#include <ECS/ECSType.hpp>
#include <ECS/ComponentManager.hpp>
#include <ECS/EntityManager.hpp>

namespace CoreEngine
{
    namespace ECS
    {
        // Facade over an entity: a lightweight handle (only the EntityID) that forwards to the ECS managers.
        // Copies refer to the same entity. Create it with ECS::CreateActor().
        class CORE_API Actor
        {
        public:
            Actor() = default;
            explicit Actor(EntityID entityID)
                : mEntityID(entityID)
            {
            }

            template <typename T, typename... Args>
            T* AddComponent(Args&&... args)
            {
                CORE_ASSERT(IsValid() && "Can't add a component to a destroyed actor!");
                return ComponentManager::GetInstance().AddComponent<T>(mEntityID, std::forward<Args>(args)...);
            }

            template <typename T>
            T* GetComponent() const
            {
                return ComponentManager::GetInstance().GetComponentByEntityID<T>(mEntityID);
            }

            template <typename T>
            bool HasComponent() const
            {
                return ComponentManager::GetInstance().HasComponent<T>(mEntityID);
            }

            template <typename T>
            void RemoveComponent()
            {
                ComponentManager::GetInstance().RemoveComponent<T>(mEntityID);
            }

            // Removes every component and recycles the id. Other copies of this actor become invalid.
            void Destroy();

            CORE_FORCE_INLINE EntityID GetID() const { return mEntityID; }
            bool IsValid() const;

            CORE_FORCE_INLINE bool operator==(const Actor& other) const { return mEntityID == other.mEntityID; }
            CORE_FORCE_INLINE bool operator!=(const Actor& other) const { return mEntityID != other.mEntityID; }

        private:
            EntityID mEntityID = INVALID_ENTITY_ID;
        };
    }
}
