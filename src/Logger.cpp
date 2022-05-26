#include <string>
#include <cstdarg>
#include <Logger.hpp>
using namespace libtasmota;

//#define PRINT_TIMESTAMP


// static initializer
Logger::ListenerEntry* Logger::s_listener = NULL;


/**
 * Constructor for logger instances
 * @param moduleName The name of the related modul - which is printed together with all log messages
 */
Logger::Logger(const char* moduleName) : m_module_name(moduleName) {
    m_module_name_w.resize(m_module_name.length());
    for (int i = 0; i < m_module_name.length(); ++i) {
        m_module_name_w[i] = m_module_name[i];
    }
}


/**
 * Add a log listener to the Logger.
 * The log listener is added globally and affects the output of all locally declared Logger instances.
 * @param listener A pointer to an object instance implementing the interface ILogListener
 * @param level The log levels that are sent to the log listener
 */
void Logger::setLogOutput(ILogListener& listener, const LogLevel level) {
    if (s_listener == NULL) {
        s_listener = new ListenerEntry();
    }
    s_listener->listener = &listener;
    s_listener->level    = level;
}


/**
 * Print a log message, where the output is in strings of char's.
 * @param level The log levels that are sent to the log listener
 * @param format The standard printf format string
 * @param ... A variable argument list
 */
void Logger::operator()(LogLevel level, const char* format, const va_list& list) const {
    std::string text;

#ifdef PRINT_TIMESTAMP
    uint64_t time = LocalHost::getUnixEpochTimeInMs();
    char strTime[32];
    snprintf(strTime, sizeof(strTime), "%015llu.%03u ", time / 1000, (unsigned)(time % 1000));
    text.append(strTime);
#endif

    switch (level) {
    case LogLevel::LOG_ERROR:
        text.append("ERROR:   ");
        break;
    case LogLevel::LOG_WARNING:
        text.append("WARNING: ");
        break;
    case LogLevel::LOG_INFO_0:
    case LogLevel::LOG_INFO_1:
    case LogLevel::LOG_INFO_2:
    case LogLevel::LOG_INFO_3:
        text.append("INFO:    ");
        break;
    default:
        break;
    }

    text.append(m_module_name);
    text.append(": ");

    char cbuf[8000];
    vsnprintf(cbuf, sizeof(cbuf)/sizeof(char), format, list);

    text.append(cbuf);
    if (text[text.size() - 1] != '\n') {
        text.append("\n");
    }

    if (s_listener != NULL && s_listener->listener != NULL) {
        if ((level & s_listener->level) != 0 || level == LogLevel::LOG_ANY) {
            s_listener->listener->operator()(text, level);
        }
    }
    else {
        fputs(text.c_str(), stderr);
    }
}


/**
 * Print a log message, where the output is in strings of char's.
 * @param level The log levels that are sent to the log listener
 * @param format The standard printf format string
 * @param ... A variable argument list
 */
void Logger::operator()(LogLevel level, const wchar_t* format, const va_list& list) const {
    std::wstring text;

#ifdef PRINT_TIMESTAMP
    uint64_t time = LocalHost::getUnixEpochTimeInMs();
    char strTime[32];
    snprintf(strTime, sizeof(strTime), "%015llu.%03u ", time / 1000, (unsigned)(time % 1000));
    text.append(strTime);
#endif

    switch (level) {
    case LogLevel::LOG_ERROR:
        text.append(L"ERROR:   ");
        break;
    case LogLevel::LOG_WARNING:
        text.append(L"WARNING: ");
        break;
    case LogLevel::LOG_INFO_0:
    case LogLevel::LOG_INFO_1:
    case LogLevel::LOG_INFO_2:
    case LogLevel::LOG_INFO_3:
        text.append(L"INFO:    ");
        break;
    default:
        break;
    }

    text.append(m_module_name_w);
    text.append(L": ");

    wchar_t cbuf[8000];
    vswprintf(cbuf, sizeof(cbuf) / sizeof(wchar_t), format, list);

    text.append(cbuf);
    if (text[text.size() - 1] != '\n') {
        text.append(L"\n");
    }

    if (s_listener != NULL && s_listener->listener != NULL) {
        if ((level & s_listener->level) != 0 || level == LogLevel::LOG_ANY) {
            s_listener->listener->operator()(text, level);
        }
    }
    else {
        fputws(text.c_str(), stderr);
    }
}
