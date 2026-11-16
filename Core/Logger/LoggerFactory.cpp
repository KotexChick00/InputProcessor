#include <Logger/LoggerFactory.hpp>

namespace CoreEngine
{
    ILoggerImplentation* LoggerFactory::Create(LoggerSpec spec, std::string loggerName)
    {
        switch (spec)
        {
        case LoggerSpec::SpdLog: return new SpdLogLoggerAdapter(loggerName);
        default:
            CORE_ASSERT("Logger spec not found");
            return nullptr;
        }
    }
}