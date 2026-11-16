#pragma once
#include <pch.h>
#include <source_location>
#include <format>

namespace CoreEngine
{
    enum class LogLevel
    {
        Trace,
        Debug,
        Warning,
        Error,
        Critical
    };

    class ILoggerImplentation
    {
    public:
        virtual ~ILoggerImplentation() = default;
        virtual void Log(
            LogLevel level,
            std::string_view message,
            const std::source_location& location) = 0;
    };

    // Format for log messages: [LEVEL] [HH:MM:SS] [Thread] Message
    class Logger
    {
    public:
        static void SetEngineImplementation(ILoggerImplentation* engineImplementation) { sEngineImplementation.reset(engineImplementation); }
        static void SetClientImplementation(ILoggerImplentation* clientImplementation) { sClientImplementation.reset(clientImplementation); }

        template <typename... Args>
        static void ClientLog(
            LogLevel level,
            const std::source_location& loc,
            std::format_string<Args...> fmt,
            Args&&... args)
        {
            if (sClientImplementation)
            {
                std::string formattedMessage = std::format(fmt, std::forward<Args>(args)...);
                sClientImplementation->Log(level, formattedMessage, loc);
            }
        }

        template <typename... Args>
        static void EngineLog(
            LogLevel level,
            const std::source_location& loc,
            std::format_string<Args...> fmt,
            Args&&... args)
        {
            if (sEngineImplementation)
            {
                std::string formattedMessage = std::format(fmt, std::forward<Args>(args)...);
                sEngineImplementation->Log(level, formattedMessage, loc);
            }
        }

    private:
        inline static Unique<ILoggerImplentation> sEngineImplementation = nullptr;
        inline static Unique<ILoggerImplentation> sClientImplementation = nullptr;

    private:
        Logger() = default;
    };

#if defined(_DEBUG) || !defined(NDEBUG)

    // --- Engine Log Macros ---
    #define IP_ENGINE_TRACE(fmt, ...)    ::CoreEngine::Logger::EngineLog(::CoreEngine::LogLevel::Trace, std::source_location::current(), fmt, ##__VA_ARGS__)
    #define IP_ENGINE_DEBUG(fmt, ...)    ::CoreEngine::Logger::EngineLog(::CoreEngine::LogLevel::Debug, std::source_location::current(), fmt, ##__VA_ARGS__)
    #define IP_ENGINE_WARN(fmt, ...)     ::CoreEngine::Logger::EngineLog(::CoreEngine::LogLevel::Warning, std::source_location::current(), fmt, ##__VA_ARGS__)
    #define IP_ENGINE_ERROR(fmt, ...)    ::CoreEngine::Logger::EngineLog(::CoreEngine::LogLevel::Error, std::source_location::current(), fmt, ##__VA_ARGS__)
    #define IP_ENGINE_CRITICAL(fmt, ...) ::CoreEngine::Logger::EngineLog(::CoreEngine::LogLevel::Critical, std::source_location::current(), fmt, ##__VA_ARGS__)

    // --- Client Log Macros ---
    #define IP_CLIENT_TRACE(fmt, ...)    ::CoreEngine::Logger::ClientLog(::CoreEngine::LogLevel::Trace, std::source_location::current(), fmt, ##__VA_ARGS__)
    #define IP_CLIENT_DEBUG(fmt, ...)    ::CoreEngine::Logger::ClientLog(::CoreEngine::LogLevel::Debug, std::source_location::current(), fmt, ##__VA_ARGS__)
    #define IP_CLIENT_WARN(fmt, ...)     ::CoreEngine::Logger::ClientLog(::CoreEngine::LogLevel::Warning, std::source_location::current(), fmt, ##__VA_ARGS__)
    #define IP_CLIENT_ERROR(fmt, ...)    ::CoreEngine::Logger::ClientLog(::CoreEngine::LogLevel::Error, std::source_location::current(), fmt, ##__VA_ARGS__)
    #define IP_CLIENT_CRITICAL(fmt, ...) ::CoreEngine::Logger::ClientLog(::CoreEngine::LogLevel::Critical, std::source_location::current(), fmt, ##__VA_ARGS__)

#else
    #define IP_ENGINE_TRACE(fmt, ...) \
        do                            \
        {                             \
        } while (0)
    #define IP_ENGINE_DEBUG(fmt, ...) \
        do                            \
        {                             \
        } while (0)
    #define IP_CLIENT_TRACE(fmt, ...) \
        do                            \
        {                             \
        } while (0)
    #define IP_CLIENT_DEBUG(fmt, ...) \
        do                            \
        {                             \
        } while (0)

    #define IP_ENGINE_WARN(fmt, ...)     ::CoreEngine::Logger::EngineLog(::CoreEngine::LogLevel::Warning, std::source_location::current(), fmt, ##__VA_ARGS__)
    #define IP_ENGINE_ERROR(fmt, ...)    ::CoreEngine::Logger::EngineLog(::CoreEngine::LogLevel::Error, std::source_location::current(), fmt, ##__VA_ARGS__)
    #define IP_ENGINE_CRITICAL(fmt, ...) ::CoreEngine::Logger::EngineLog(::CoreEngine::LogLevel::Critical, std::source_location::current(), fmt, ##__VA_ARGS__)

    #define IP_CLIENT_WARN(fmt, ...)     ::CoreEngine::Logger::ClientLog(::CoreEngine::LogLevel::Warning, std::source_location::current(), fmt, ##__VA_ARGS__)
    #define IP_CLIENT_ERROR(fmt, ...)    ::CoreEngine::Logger::ClientLog(::CoreEngine::LogLevel::Error, std::source_location::current(), fmt, ##__VA_ARGS__)
    #define IP_CLIENT_CRITICAL(fmt, ...) ::CoreEngine::Logger::ClientLog(::CoreEngine::LogLevel::Critical, std::source_location::current(), fmt, ##__VA_ARGS__)

#endif
}