#include <Logger/SpdLog/SpdLogLoggerAdapter.hpp>

namespace CoreEngine::Logger::SpdLog {
    SpdLogLoggerAdapter::SpdLogLoggerAdapter(const std::string& loggerName) {
        auto consoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        consoleSink->set_pattern("%^[%H:%M:%S] [%s:%#] [%n] [Thread %t]: %v%$");
        mLogger = std::make_unique<spdlog::logger>(loggerName, consoleSink);

        mLogger->set_level(spdlog::level::trace);
        mLogger->flush_on(spdlog::level::err);
    }

    void SpdLogLoggerAdapter::Log(
        LogLevel level,
        std::string_view message,
        const std::source_location& location
    ) {
        if (!mLogger) return;

        // Mapping enum
        spdlog::level::level_enum spdLevel = spdlog::level::info;
        switch (level) {
        case LogLevel::Trace:    spdLevel = spdlog::level::trace;    break;
        case LogLevel::Debug:    spdLevel = spdlog::level::debug;    break;
        case LogLevel::Warning:  spdLevel = spdlog::level::warn;     break;
        case LogLevel::Error:    spdLevel = spdlog::level::err;      break;
        case LogLevel::Critical: spdLevel = spdlog::level::critical; break;
        }

        spdlog::source_loc loc{
            location.file_name(),
            static_cast<int>(location.line()),
            location.function_name()
        };

        mLogger->log(loc, spdLevel, spdlog::string_view_t(message.data(), message.size()));
    }
}