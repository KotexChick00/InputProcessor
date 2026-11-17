#pragma once
#include <pch.h>
#include <ECS/ECSType.hpp>

namespace CoreEngine
{
    namespace ECS
    {
        template <typename T>
        class ComponentContainer;

        // Base of every component. The owner is set when the component is added to an entity.
        class CORE_API IComponent
        {
        public:
            virtual ~IComponent() = default;

            CORE_FORCE_INLINE EntityID GetOwner() const { return mOwner; }

        private:
            template <typename T>
            friend class ComponentContainer;

            EntityID mOwner = INVALID_ENTITY_ID;
        };
    }
}
