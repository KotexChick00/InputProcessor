#include <ECS/ECS.hpp>

namespace CoreEngine
{
    namespace ECS
    {
        Actor CreateActor()
        {
            return Actor(EntityManager::GetInstance().Create());
        }

        void DestroyActor(Actor& actor)
        {
            actor.Destroy();
        }

        Actor GetActor(EntityID entityID)
        {
            return EntityManager::GetInstance().IsAlive(entityID) ? Actor(entityID) : Actor();
        }

        void Update(float deltaTime)
        {
            SystemManager::GetInstance().Update(deltaTime);
        }

        void Shutdown()
        {
            // Systems may still read components in OnDestroy, so they go first.
            SystemManager::GetInstance().Clear();
            ComponentManager::GetInstance().Clear();
            EntityManager::GetInstance().Clear();
        }
    }
}
