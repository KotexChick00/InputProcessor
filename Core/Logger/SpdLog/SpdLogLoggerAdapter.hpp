#pragma once
#include <iostream>
#include <Logger/Logger.hpp>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

namespace InputProcessor::Logger::SpdLog {
	class SpdLogLoggerAdapter : public InputProcessor::Logger::ILoggerImplentation {
	public:
		SpdLogLoggerAdapter(const std::string& loggerName);

		void Log(
			LogLevel level,
			std::string_view message,
			const std::source_location& location
		);
	private:
		inline static std::shared_ptr<spdlog::logger> mLogger = nullptr;
	};
}