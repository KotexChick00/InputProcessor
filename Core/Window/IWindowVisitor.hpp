#pragma once
#include <Window/IWindow.hpp>

// Forward-declare WxWindow to avoid pulling in all wxWidgets headers
// into every visitor (they are only needed in .cpp files that use WxWindow).
namespace CoreEngine
{
    class WxWindow;
}

namespace CoreEngine
{
    class CORE_API IWindowVisitor
    {
    public:
        virtual void Visit(IWindow* window) = 0;
        virtual void VisitWxWindow(WxWindow* window) = 0;
    };
}
