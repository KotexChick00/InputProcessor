#include <ECS/ComponentManager.hpp>

namespace CoreEngine
{
    namespace ECS
    {
        ComponentManager& ComponentManager::GetInstance()
        {
            static ComponentManager sInstance;
            return sInstance;
        }

        void ComponentManager::RemoveAllComponents(EntityID entityID)
        {
            for (auto& [typeID, container] : mContainers)
            {
                if (container)
                {
                    container->Remove(entityID);
                }
            }
        }

        void ComponentManager::Clear()
        {
            mContainers.clear();
        }
    }
}
