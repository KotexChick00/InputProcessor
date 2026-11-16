#pragma once
#include <pch.h>
#include <Renderer/IRenderer.hpp>

namespace CoreEngine
{
    enum class CORE_API RendererSpec
    {
        Opengl,
        Vulkan,
        DirectX
    };

    class RendererFactory
    {
    public:
        static IRenderer* Create(RendererSpec spec);

    private:
        RendererFactory() = default;
        ~RendererFactory() = default;
    };
}