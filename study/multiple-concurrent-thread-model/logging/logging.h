#pragma once
#include <string>
#include <cstring>
#include <sstream>

namespace mctm
{
    enum LogSeverity
    {
        LOG_VERBOSE = -1,
        LOG_INFO = 0,
        LOG_WARNING = 1,
        LOG_ERROR = 2,
        LOG_FATAL = 3,
        LOG_NUM_SEVERITIES = 4,
    };

    class LogMessage
    {
    public:
        // Used for LOG(severity).
        LogMessage(const char* file, int line, LogSeverity severity);

        //// Used for CHECK_EQ(), etc. Takes ownership of the given string.
        //// Implied severity = LOG_FATAL.
        //LogMessage(const char* file, int line, std::string* result);

        //// Used for DCHECK_EQ(), etc. Takes ownership of the given string.
        //LogMessage(const char* file,
        //    int line,
        //    LogSeverity severity,
        //    std::string* result);

        ~LogMessage();

        std::ostream& stream() { return stream_; }

    private:
        void Init();

    private:
        std::ostringstream stream_;
        LogSeverity severity_;

        // The file and line information passed in to the constructor.
        const char* file_ = nullptr;
        const int line_ = 0;
    };

    class LogMessageVoidify
    {
    public:
        LogMessageVoidify() {}
        // This has to be an operator with a precedence lower than << but
        // higher than ?:
        void operator&(std::ostream&) {}
    };
}

#ifndef DCHECK

#if defined(_DEBUG) || defined(DEBUG)
#define DCHECK_IS_ON() 1
#else
#define DCHECK_IS_ON() 0
#endif

#define COMPACT_MCTM_LOG_FATAL(ClassName, ...)\
    mctm::ClassName(__FILE__, __LINE__, mctm::LOG_FATAL, ##__VA_ARGS__)

#define COMPACT_MCTM_LOG_DCHECK COMPACT_MCTM_LOG_FATAL(LogMessage)

#define LOG_STREAM(severity) COMPACT_MCTM_LOG_##severity.stream()

#define LAZY_STREAM(stream, condition) \
    !(condition) ? (void)0 : mctm::LogMessageVoidify() & (stream)

#define DCHECK(condition)                                         \
    LAZY_STREAM(LOG_STREAM(DCHECK), DCHECK_IS_ON() && !(condition)) \
    << "Check failed: " #condition ". "

#endif