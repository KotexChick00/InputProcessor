#pragma once
#include <pch.h>
#include <ECS/ECSType.hpp>
#include <ECS/Actor.hpp>
#include <ECS/ComponentManager.hpp>
#include <ECS/EntityManager.hpp>
#include <ECS/SystemManager.hpp>

// Entry point of the Entity Component System: include this header and use ECS::CreateActor().
namespace CoreEngine
{
    namespace ECS
    {
        CORE_API Actor CreateActor();
        // Removes every component of the actor and recycles its id.
        CORE_API void DestroyActor(Actor& actor);
        // Actor handle for an existing entity; invalid if the entity isn't alive.
        CORE_API Actor GetActor(EntityID entityID);

        // Runs every system in dependency order.
        CORE_API void Update(float deltaTime);

        // Destroys every system, component and entity. Call it before the logger shuts down:
        // the singletons are otherwise destroyed in an unspecified order at program exit.
        CORE_API void Shutdown();
    }
}
