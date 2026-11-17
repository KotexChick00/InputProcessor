#include <ECS/SystemManager.hpp>
#include <string_view>
#include <tuple>

namespace CoreEngine
{
    namespace ECS
    {
        SystemManager& SystemManager::GetInstance()
        {
            static SystemManager sInstance;
            return sInstance;
        }

        void SystemManager::RegisterSystem(Unique<ISystem> system, SystemTypeID typeID, const char* typeName)
        {
            // MSVC names types "class Foo" / "struct Foo": keep only the type name for logs.
            std::string_view name = typeName;
            for (std::string_view prefix : {std::string_view("class "), std::string_view("struct ")})
            {
                if (name.starts_with(prefix))
                {
                    name.remove_prefix(prefix.size());
                }
            }

            system->mTypeID = typeID;
            system->mRegistrationIndex = mNextRegistrationIndex++;
            system->mName = std::string(name);

            if (typeID >= mSystems.size())
            {
                mSystems.resize(typeID + 1);
            }

            ISystem* rawSystem = system.get();
            mSystems[typeID] = std::move(system);
            mOrderDirty = true;

            rawSystem->OnCreate();
        }

        void SystemManager::RemoveSystem(SystemTypeID typeID)
        {
            if (typeID >= mSystems.size() || !mSystems[typeID])
            {
                return;
            }

            mSystems[typeID]->OnDestroy();
            mSystems[typeID].reset();
            mOrderDirty = true;
        }

        void SystemManager::AddDependency(SystemTypeID systemTypeID, SystemTypeID dependencyTypeID)
        {
            if (systemTypeID == dependencyTypeID)
            {
                IP_ENGINE_ERROR("A system can't depend on itself!");
                return;
            }

            if (mDependencies[systemTypeID].insert(dependencyTypeID).second)
            {
                mOrderDirty = true;
            }
        }

        void SystemManager::Update(float deltaTime)
        {
            if (mOrderDirty)
            {
                BuildExecutionOrder();
            }

            // Look each system up by id: a system may remove another one while updating.
            for (SystemTypeID typeID : mExecutionOrder)
            {
                ISystem* system = typeID < mSystems.size() ? mSystems[typeID].get() : nullptr;
                if (system && system->IsEnabled())
                {
                    system->OnUpdate(deltaTime);
                }
            }
        }

        void SystemManager::Clear()
        {
            if (mOrderDirty)
            {
                BuildExecutionOrder();
            }

            // Dependents are destroyed before what they depend on.
            for (auto typeID = mExecutionOrder.rbegin(); typeID != mExecutionOrder.rend(); ++typeID)
            {
                RemoveSystem(*typeID);
            }
            // Systems stuck in a cycle are not in the execution order.
            for (SystemTypeID typeID = 0; typeID < mSystems.size(); ++typeID)
            {
                RemoveSystem(typeID);
            }

            mSystems.clear();
            mDependencies.clear();
            mExecutionOrder.clear();
            mNextRegistrationIndex = 0;
            mOrderDirty = true;
            mHasDependencyCycle = false;
        }

        size_t SystemManager::GetSystemCount() const
        {
            return std::count_if(mSystems.begin(), mSystems.end(), [](const Unique<ISystem>& system) { return system != nullptr; });
        }

        void SystemManager::BuildExecutionOrder()
        {
            // Kahn's algorithm over the added systems. Dependencies on systems that aren't added are ignored.
            std::unordered_map<SystemTypeID, size_t> inDegree;
            std::unordered_map<SystemTypeID, std::vector<SystemTypeID>> dependents;
            for (const Unique<ISystem>& system : mSystems)
            {
                if (!system)
                {
                    continue;
                }

                size_t& degree = inDegree[system->mTypeID];
                auto dependencies = mDependencies.find(system->mTypeID);
                if (dependencies == mDependencies.end())
                {
                    continue;
                }

                for (SystemTypeID dependencyTypeID : dependencies->second)
                {
                    if (dependencyTypeID < mSystems.size() && mSystems[dependencyTypeID])
                    {
                        ++degree;
                        dependents[dependencyTypeID].push_back(system->mTypeID);
                    }
                }
            }

            // Ready systems: lowest priority value first, then the earliest registered.
            using ReadyEntry = std::tuple<int, size_t, SystemTypeID>;
            std::priority_queue<ReadyEntry, std::vector<ReadyEntry>, std::greater<ReadyEntry>> ready;
            auto pushReady = [&](SystemTypeID typeID) {
                const ISystem* system = mSystems[typeID].get();
                ready.emplace(static_cast<int>(system->mPriority), system->mRegistrationIndex, typeID);
            };

            for (const auto& [typeID, degree] : inDegree)
            {
                if (degree == 0)
                {
                    pushReady(typeID);
                }
            }

            mExecutionOrder.clear();
            while (!ready.empty())
            {
                SystemTypeID typeID = std::get<2>(ready.top());
                ready.pop();
                mExecutionOrder.push_back(typeID);

                for (SystemTypeID dependentTypeID : dependents[typeID])
                {
                    if (--inDegree[dependentTypeID] == 0)
                    {
                        pushReady(dependentTypeID);
                    }
                }
            }

            mHasDependencyCycle = mExecutionOrder.size() < inDegree.size();
            if (mHasDependencyCycle)
            {
                std::string blockedSystems;
                for (const auto& [typeID, degree] : inDegree)
                {
                    if (degree > 0)
                    {
                        blockedSystems += (blockedSystems.empty() ? "" : ", ") + mSystems[typeID]->mName;
                    }
                }
                IP_ENGINE_ERROR("System dependency cycle detected, these systems won't run: {}", blockedSystems);
            }

            mOrderDirty = false;
        }
    }
}
