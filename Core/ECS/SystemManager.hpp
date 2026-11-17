#pragma once
#include <pch.h>
#include <ECS/ECSType.hpp>
#include <ECS/ISystem.hpp>
#include <Logger/Logger.hpp>
#include <typeinfo>

namespace CoreEngine
{
    namespace ECS
    {
        struct SystemFamily;
        using SystemTypeIDGenerator = TypeIDGenerator<SystemFamily>;

        // Owns the systems and runs them in a topological order of their dependencies (Kahn's algorithm).
        // Systems without a dependency between them run by ESystemPriority, then by registration order.
        class CORE_API SystemManager
        {
        public:
            static SystemManager& GetInstance();

            SystemManager(const SystemManager&) = delete;
            SystemManager& operator=(const SystemManager&) = delete;

            template <typename T>
            static SystemTypeID GetSystemTypeID()
            {
                return SystemTypeIDGenerator::Get<T>();
            }

            // Creates the system and calls OnCreate. Returns the existing one if T is already added.
            template <typename T, typename... Args>
            T* AddSystem(Args&&... args)
            {
                CORE_STATIC_ASSERT(std::is_base_of_v<ISystem, T>, "System type must derive from ISystem!");

                if (T* existing = GetSystem<T>())
                {
                    IP_ENGINE_WARN("System {} is already added, the existing one is returned!", existing->GetName());
                    return existing;
                }

                Unique<T> system = std::make_unique<T>(std::forward<Args>(args)...);
                T* result = system.get();
                RegisterSystem(std::move(system), GetSystemTypeID<T>(), typeid(T).name());
                return result;
            }

            template <typename T>
            T* GetSystem() const
            {
                SystemTypeID typeID = GetSystemTypeID<T>();
                return typeID < mSystems.size() ? static_cast<T*>(mSystems[typeID].get()) : nullptr;
            }

            // Calls OnDestroy and deletes the system. Its dependencies are kept for when it is added again.
            template <typename T>
            void RemoveSystem()
            {
                RemoveSystem(GetSystemTypeID<T>());
            }

            // TDependency runs before TSystem. Both may be added before or after this call.
            template <typename TSystem, typename TDependency>
            void AddDependency()
            {
                AddDependency(GetSystemTypeID<TSystem>(), GetSystemTypeID<TDependency>());
            }

            // Rebuilds the execution order if needed, then updates every enabled system.
            void Update(float deltaTime);

            // Calls OnDestroy in reverse execution order and removes every system and dependency.
            void Clear();

            // True when the last order build found a dependency cycle: the systems in the cycle
            // (and the ones depending on them) don't run.
            CORE_FORCE_INLINE bool HasDependencyCycle() const { return mHasDependencyCycle; }
            CORE_FORCE_INLINE const std::vector<SystemTypeID>& GetExecutionOrder() const { return mExecutionOrder; }
            size_t GetSystemCount() const;

            void RemoveSystem(SystemTypeID typeID);
            void AddDependency(SystemTypeID systemTypeID, SystemTypeID dependencyTypeID);

        private:
            SystemManager() = default;
            ~SystemManager() = default;

            void RegisterSystem(Unique<ISystem> system, SystemTypeID typeID, const char* typeName);
            void BuildExecutionOrder();

        private:
            std::vector<Unique<ISystem>> mSystems;                                            // Indexed by SystemTypeID, nullptr when not added
            std::unordered_map<SystemTypeID, std::unordered_set<SystemTypeID>> mDependencies; // System -> systems that run before it
            std::vector<SystemTypeID> mExecutionOrder;
            size_t mNextRegistrationIndex = 0;
            bool mOrderDirty = true;
            bool mHasDependencyCycle = false;
        };
    }
}
