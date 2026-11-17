#include <ECS/EntityManager.hpp>
#include <Logger/Logger.hpp>

namespace CoreEngine
{
    namespace ECS
    {
        EntityManager& EntityManager::GetInstance()
        {
            static EntityManager sInstance;
            return sInstance;
        }

        EntityID EntityManager::Create()
        {
            EntityID entityID = INVALID_ENTITY_ID;
            if (!mRecycledIDs.empty())
            {
                entityID = mRecycledIDs.front();
                mRecycledIDs.pop();
            }
            else
            {
                CORE_ASSERT(mNextID != INVALID_ENTITY_ID && "Out of entity ids!");
                entityID = mNextID++;
                mAlive.push_back(false);
            }

            mAlive[entityID] = true;
            ++mAliveCount;
            return entityID;
        }

        void EntityManager::Destroy(EntityID entityID)
        {
            if (!IsAlive(entityID))
            {
                IP_ENGINE_WARN("Can't destroy entity {} because it isn't alive!", entityID);
                return;
            }

            mAlive[entityID] = false;
            mRecycledIDs.push(entityID);
            --mAliveCount;
        }

        bool EntityManager::IsAlive(EntityID entityID) const
        {
            return entityID < mAlive.size() && mAlive[entityID];
        }

        void EntityManager::Clear()
        {
            mAlive.clear();
            mRecycledIDs = {};
            mNextID = 0;
            mAliveCount = 0;
        }
    }
}
