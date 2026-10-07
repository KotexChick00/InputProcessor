#pragma once
#include <pch.h>
#include <Logger/Logger.hpp>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#define SPDLOG_COMPILED_LIB
namespace CoreEngine::Logger::SpdLog {
	class SpdLogLoggerAdapter : public ILoggerImplentation {
	public:
		SpdLogLoggerAdapter(const std::string& loggerName);

		void Log(
			LogLevel level,
			std::string_view message,
			const std::source_location& location
		);
	private:
		Unique<spdlog::logger> mLogger = nullptr;
	};
}