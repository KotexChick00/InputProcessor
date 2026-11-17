#pragma once
#include <pch.h>
#include <ECS/ECSType.hpp>
#include <ECS/ComponentContainer.hpp>

namespace CoreEngine
{
    namespace ECS
    {
        struct ComponentFamily;
        using ComponentTypeIDGenerator = TypeIDGenerator<ComponentFamily>;

        // Owns one ComponentContainer per component type, keyed by ComponentTypeID.
        class CORE_API ComponentManager
        {
        public:
            static ComponentManager& GetInstance();

            ComponentManager(const ComponentManager&) = delete;
            ComponentManager& operator=(const ComponentManager&) = delete;

            template <typename T>
            static ComponentTypeID GetComponentTypeID()
            {
                return ComponentTypeIDGenerator::Get<T>();
            }

            template <typename T, typename... Args>
            T* AddComponent(EntityID entityID, Args&&... args)
            {
                CORE_ASSERT(entityID != INVALID_ENTITY_ID && "Can't add a component to an invalid entity!");
                return GetOrCreateContainer<T>()->Add(entityID, std::forward<Args>(args)...);
            }

            template <typename T>
            void RemoveComponent(EntityID entityID)
            {
                if (ComponentContainer<T>* container = GetContainer<T>())
                {
                    container->Remove(entityID);
                }
            }

            template <typename T>
            bool HasComponent(EntityID entityID) const
            {
                const ComponentContainer<T>* container = GetContainer<T>();
                return container && container->Has(entityID);
            }

            // nullptr when the entity has no component of type T.
            template <typename T>
            T* GetComponentByEntityID(EntityID entityID) const
            {
                const ComponentContainer<T>* container = GetContainer<T>();
                return container ? container->Get(entityID) : nullptr;
            }

            // Every component of type T. Don't add or remove components of type T while iterating.
            template <typename T>
            ComponentView<T> GetAllComponents() const
            {
                const ComponentContainer<T>* container = GetContainer<T>();
                return container ? container->GetView() : ComponentView<T>();
            }

            template <typename T>
            size_t GetComponentCount() const
            {
                const ComponentContainer<T>* container = GetContainer<T>();
                return container ? container->GetCount() : 0;
            }

            // Removes the components of every type owned by the entity (used when the entity is destroyed).
            void RemoveAllComponents(EntityID entityID);
            // Destroys every component and container.
            void Clear();

        private:
            ComponentManager() = default;
            ~ComponentManager() = default;

            template <typename T>
            ComponentContainer<T>* GetContainer() const
            {
                auto found = mContainers.find(GetComponentTypeID<T>());
                if (found == mContainers.end())
                {
                    return nullptr;
                }
                return static_cast<ComponentContainer<T>*>(found->second.get());
            }

            template <typename T>
            ComponentContainer<T>* GetOrCreateContainer()
            {
                Unique<IComponentContainer>& container = mContainers[GetComponentTypeID<T>()];
                if (!container)
                {
                    container = std::make_unique<ComponentContainer<T>>();
                }
                return static_cast<ComponentContainer<T>*>(container.get());
            }

        private:
            std::unordered_map<ComponentTypeID, Unique<IComponentContainer>> mContainers;
        };
    }
}
