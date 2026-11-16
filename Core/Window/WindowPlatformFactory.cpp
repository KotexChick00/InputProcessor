#include <Window/WindowPlatformFactory.hpp>
#include <pch.h>

#include <Window/Wx/WxWindow.hpp>

namespace CoreEngine
{
    IWindow* WindowPlatformFactory::Create(WindowPlatformSpec spec)
    {
        switch (spec)
        {
        case WindowPlatformSpec::Wx: return new WxWindow();
        case WindowPlatformSpec::SDL:
            CORE_ASSERT(false && "SDL Platform currently doesn't support");
            break;
        case WindowPlatformSpec::Injection:
            CORE_ASSERT(false && "Injected windows are supplied by the caller, not the factory");
            break;
        default:
            CORE_ASSERT(false && "Didn't found the platform");
            break;
        }
        return nullptr;
    }
}
