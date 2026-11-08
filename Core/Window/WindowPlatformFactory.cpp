#include <Window/WindowPlatformFactory.hpp>
#include <pch.h>

#include <Window/Wx/WxWindow.hpp>

namespace CoreEngine::Window {
        IWindow* WindowPlatformFactory::Create(WindowPlatformSpec spec) {
                switch (spec) {
                case WindowPlatformSpec::Wx:   return new Wx::WxWindow();
                case WindowPlatformSpec::SDL:
                        CORE_ASSERT(false && "SDL Platform currently doesn't support");
                        break;
                default:
                        CORE_ASSERT(false && "Didn't found the platform");
                        break;
                }
                return nullptr;
        }
}
