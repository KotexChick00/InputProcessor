#include <Logger/LoggerFactory.hpp>

namespace CoreEngine::Logger {
	ILoggerImplentation* LoggerFactory::Create(LoggerSpec spec, std::string loggerName) {
		switch (spec) {
		case LoggerSpec::SpdLog: return new SpdLog::SpdLogLoggerAdapter(loggerName);
		default:
			CORE_ASSERT("Logger spec not found");
			return nullptr;
		}
	}
}