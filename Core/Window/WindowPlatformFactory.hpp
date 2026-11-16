#pragma once
#include <Window/IWindow.hpp>

namespace CoreEngine
{
    enum class CORE_API WindowPlatformSpec
    {
        SDL,
        Wx,
        Injection // The application uses ApplicationConfiguration::InjectedWindow instead of the factory
    };

    class WindowPlatformFactory
    {
    public:
        static IWindow* Create(WindowPlatformSpec spec);

    private:
        WindowPlatformFactory() = default;
        ~WindowPlatformFactory() = default;
        WindowPlatformFactory(WindowPlatformFactory&) = default;
    };
}