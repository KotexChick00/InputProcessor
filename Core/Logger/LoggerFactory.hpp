#pragma once
#include <Logger/Logger.hpp>
#include <Logger/SpdLog/SpdLogLoggerAdapter.hpp>
#include <pch.h>

namespace CoreEngine::Logger {
	enum class LoggerSpec {
		SpdLog
	};
	
	class LoggerFactory {
	public:
		static ILoggerImplentation* Create(LoggerSpec spec, std::string loggerName);

	private:
		LoggerFactory() = default;
		~LoggerFactory() = default;
	};
}