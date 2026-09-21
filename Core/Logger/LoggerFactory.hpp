#pragma once
#include <pch.h>
#include <Logger/Logger.hpp>
#include <Logger/SpdLog/SpdLogLoggerAdapter.hpp>

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