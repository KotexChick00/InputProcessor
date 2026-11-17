#include <ECS/Actor.hpp>
#include <Logger/Logger.hpp>

namespace CoreEngine
{
    namespace ECS
    {
        void Actor::Destroy()
        {
            if (!IsValid())
            {
                IP_ENGINE_WARN("Can't destroy actor {} because it isn't alive!", mEntityID);
                return;
            }

            ComponentManager::GetInstance().RemoveAllComponents(mEntityID);
            EntityManager::GetInstance().Destroy(mEntityID);
            mEntityID = INVALID_ENTITY_ID;
        }

        bool Actor::IsValid() const
        {
            return EntityManager::GetInstance().IsAlive(mEntityID);
        }
    }
}
