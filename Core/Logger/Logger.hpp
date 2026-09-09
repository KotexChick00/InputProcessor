#pragma once
#include <iostream>
#include <source_location>
#include <format>

namespace InputProcessor::Logger {
	enum class LogLevel {
		Trace,
		Debug,
		Warning,
		Error,
		Critical
	};
	
	class ILoggerImplentation {
	public:
		virtual ~ILoggerImplentation() = default;
		virtual void Log(
			LogLevel level, 
			std::string_view message, 
			const std::source_location& location
		) = 0;
	};
	
	// Format for log messages: [LEVEL] [HH:MM:SS] [Thread] Message
	class Logger {
	public:
		static void SetEngineImplementation(std::shared_ptr<ILoggerImplentation> engineImplementation) { s_EngineImplementation = engineImplementation; }
		static void SetClientImplementation(std::shared_ptr<ILoggerImplentation> clientImplementation) { s_ClientImplementation = clientImplementation; }

		template <typename... Args>
		static void ClientLog(
			LogLevel level,
			const std::source_location& loc,
			std::format_string<Args...> fmt,
			Args&&... args
		) {
			if (s_ClientImplementation) {
				std::string formatted_message = std::format(fmt, std::forward<Args>(args)...);
				s_ClientImplementation->Log(level, formatted_message, loc);
			}
		}
		
		template <typename... Args>
		static void EngineLog(
			LogLevel level,
			const std::source_location& loc,
			std::format_string<Args...> fmt,
			Args&&... args
		) {
			if (s_EngineImplementation) {
				std::string formatted_message = std::format(fmt, std::forward<Args>(args)...);
				s_EngineImplementation->Log(level, formatted_message, loc);
			}
		}

	private:
		inline static std::shared_ptr<ILoggerImplentation> s_EngineImplementation = nullptr;
		inline static std::shared_ptr<ILoggerImplentation> s_ClientImplementation = nullptr;

	private:
		Logger() = default;
	};

#if defined(_DEBUG) || !defined(NDEBUG)

	// --- Engine Log Macros ---
#define IP_ENGINE_TRACE(fmt, ...)    ::InputProcessor::Logger::Logger::EngineLog(::InputProcessor::Logger::LogLevel::Trace,    std::source_location::current(), fmt, ##__VA_ARGS__)
#define IP_ENGINE_DEBUG(fmt, ...)    ::InputProcessor::Logger::Logger::EngineLog(::InputProcessor::Logger::LogLevel::Debug,    std::source_location::current(), fmt, ##__VA_ARGS__)
#define IP_ENGINE_WARN(fmt, ...)     ::InputProcessor::Logger::Logger::EngineLog(::InputProcessor::Logger::LogLevel::Warning,  std::source_location::current(), fmt, ##__VA_ARGS__)
#define IP_ENGINE_ERROR(fmt, ...)    ::InputProcessor::Logger::Logger::EngineLog(::InputProcessor::Logger::LogLevel::Error,    std::source_location::current(), fmt, ##__VA_ARGS__)
#define IP_ENGINE_CRITICAL(fmt, ...) ::InputProcessor::Logger::Logger::EngineLog(::InputProcessor::Logger::LogLevel::Critical, std::source_location::current(), fmt, ##__VA_ARGS__)

// --- Client Log Macros ---
#define IP_CLIENT_TRACE(fmt, ...)    ::InputProcessor::Logger::Logger::ClientLog(::InputProcessor::Logger::LogLevel::Trace,    std::source_location::current(), fmt, ##__VA_ARGS__)
#define IP_CLIENT_DEBUG(fmt, ...)    ::InputProcessor::Logger::Logger::ClientLog(::InputProcessor::Logger::LogLevel::Debug,    std::source_location::current(), fmt, ##__VA_ARGS__)
#define IP_CLIENT_WARN(fmt, ...)     ::InputProcessor::Logger::Logger::ClientLog(::InputProcessor::Logger::LogLevel::Warning,  std::source_location::current(), fmt, ##__VA_ARGS__)
#define IP_CLIENT_ERROR(fmt, ...)    ::InputProcessor::Logger::Logger::ClientLog(::InputProcessor::Logger::LogLevel::Error,    std::source_location::current(), fmt, ##__VA_ARGS__)
#define IP_CLIENT_CRITICAL(fmt, ...) ::InputProcessor::Logger::Logger::ClientLog(::InputProcessor::Logger::LogLevel::Critical, std::source_location::current(), fmt, ##__VA_ARGS__)

#else
#define IP_ENGINE_TRACE(fmt, ...)    do {} while(0)
#define IP_ENGINE_DEBUG(fmt, ...)    do {} while(0)
#define IP_CLIENT_TRACE(fmt, ...)    do {} while(0)
#define IP_CLIENT_DEBUG(fmt, ...)    do {} while(0)

#define IP_ENGINE_WARN(fmt, ...)     ::InputProcessor::Logger::Logger::EngineLog(::InputProcessor::Logger::LogLevel::Warning,  std::source_location::current(), fmt, ##__VA_ARGS__)
#define IP_ENGINE_ERROR(fmt, ...)    ::InputProcessor::Logger::Logger::EngineLog(::InputProcessor::Logger::LogLevel::Error,    std::source_location::current(), fmt, ##__VA_ARGS__)
#define IP_ENGINE_CRITICAL(fmt, ...) ::InputProcessor::Logger::Logger::EngineLog(::InputProcessor::Logger::LogLevel::Critical, std::source_location::current(), fmt, ##__VA_ARGS__)

#define IP_CLIENT_WARN(fmt, ...)     ::InputProcessor::Logger::Logger::ClientLog(::InputProcessor::Logger::LogLevel::Warning,  std::source_location::current(), fmt, ##__VA_ARGS__)
#define IP_CLIENT_ERROR(fmt, ...)    ::InputProcessor::Logger::Logger::ClientLog(::InputProcessor::Logger::LogLevel::Error,    std::source_location::current(), fmt, ##__VA_ARGS__)
#define IP_CLIENT_CRITICAL(fmt, ...) ::InputProcessor::Logger::Logger::ClientLog(::InputProcessor::Logger::LogLevel::Critical, std::source_location::current(), fmt, ##__VA_ARGS__)

#endif
}