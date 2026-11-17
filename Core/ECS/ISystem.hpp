#pragma once
#include <pch.h>
#include <ECS/ECSType.hpp>
#include <string>

namespace CoreEngine
{
    namespace ECS
    {
        class SystemManager;

        // Base of every system. Systems are created and run by the SystemManager.
        class CORE_API ISystem
        {
        public:
            explicit ISystem(ESystemPriority priority = ESystemPriority::Medium)
                : mPriority(priority)
            {
            }
            virtual ~ISystem() = default;

            virtual void OnCreate() { }
            virtual void OnUpdate(float deltaTime) = 0;
            virtual void OnDestroy() { }

            CORE_FORCE_INLINE ESystemPriority GetPriority() const { return mPriority; }
            CORE_FORCE_INLINE SystemTypeID GetTypeID() const { return mTypeID; }
            CORE_FORCE_INLINE const std::string& GetName() const { return mName; }
            CORE_FORCE_INLINE bool IsEnabled() const { return mEnabled; }
            CORE_FORCE_INLINE void SetEnabled(bool enabled) { mEnabled = enabled; }

        private:
            friend class SystemManager;

            ESystemPriority mPriority = ESystemPriority::Medium;
            SystemTypeID mTypeID = 0;
            size_t mRegistrationIndex = 0;
            std::string mName;
            bool mEnabled = true;
        };
    }
}
