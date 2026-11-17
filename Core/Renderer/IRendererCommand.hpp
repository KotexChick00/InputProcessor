#pragma once
#include <Renderer/RendererPrimitives.hpp>
#include <pch.h>

namespace CoreEngine
{

    class CORE_API IRendererCommand
    {
    public:
        virtual void ClearBuffers(ClearBufferMasks clearBufferMasks) = 0;
        virtual void Draw(RenderMode renderMode, unsigned int count, unsigned int start = 0) = 0;
        virtual void DrawIndex(RenderMode renderMode, unsigned int count, unsigned int start = 0) = 0;
        // Applies to both front and back faces until changed again.
        virtual void SetPolygonMode(PolygonMode polygonMode) = 0;
    };
}