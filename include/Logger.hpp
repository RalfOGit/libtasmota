#ifndef _LIBTASMOTA_LOGGER_HPP_
#define _LIBTASMOTA_LOGGER_HPP_

#include <string>
#include <cstdarg>

#ifdef LIB_NAMESPACE
namespace LIB_NAMESPACE {
#else
namespace libtasmota {
#endif

    /*! \file */
    /**
     *   Enumeration describing the defined log levels.
     */
    enum class LogLevel {
        LOG_ANY     = 0x00,     /**< don't care level*/
        LOG_ERROR   = 0x01,     /**< error level     */
        LOG_WARNING = 0x02,     /**< warning level   */
        LOG_INFO_0  = 0x04,     /**< verbose level 0 */
        LOG_INFO_1  = 0x08,     /**< verbose level 1 */
        LOG_INFO_2  = 0x10,     /**< verbose level 2 */
        LOG_INFO_3  = 0x20,     /**< verbose level 3 */
    };


    /** Global scope operator for bitwise or'ing two LogLevel enum values */
    inline LogLevel operator|(const LogLevel& op1, const LogLevel& op2) {
        return (LogLevel)((int)op1 | (int)op2);
    }

    /** Global scope operator for bitwise and'ing two LogLevel enum values */
    inline LogLevel operator&(const LogLevel& op1, const LogLevel& op2) {
        return (LogLevel)((int)op1 & (int)op2);
    }

    /** Global scope operator for not equal comparison of two LogLevel enum values */
    inline bool operator!=(const LogLevel& op1, const int& op2) {
        return ((int)op1 != (int)op2);
    }

    /** Global scope operator for equal comparison of two LogLevel enum values */
    inline bool operator==(const LogLevel& op1, const int& op2) {
        return ((int)op1 == (int)op2);
    }


    /**
     *  Interface for routing log messages created by class Logger.
     *  Classes implementing this interface can route logger output to e.g. stdout, stderr, files, ...
     */
    class ILogListener {

    public:
        /** Virtual destructor */
        virtual ~ILogListener(void) {}

        /**
         *  Output a single byte character message.
         *  @param msg The message string
         *  @param level The log level of the message string
         */
        virtual void operator()(const std::string& msg, const LogLevel& level) = 0;

        /**
         *  Output a wide character message.
         *  @param msg The message string
         *  @param level The log level of the message string
         */
        virtual void operator()(const std::wstring& msg, const LogLevel& level) {}
    };


    /**
     *  Logger class.
     *  An instance of this classed can be instanciated for each module, for instance by declaring a static instance with local scope inside
     *  the source code of the module. The class supports different log levels (see class LogLevel) and the registration of log listeners
     *  (see interface ILogListener). Log listeners can route log output to different output means and also limit the output to the
     *  log levels defined during registration.
     */
    class Logger {

    public:
        Logger(const char* moduleName);

        static void setLogOutput(ILogListener& listener, const LogLevel level);

        template <class CharType> void error     (const CharType* format, ...) const;
        template <class CharType> void warning   (const CharType* format, ...) const;
        template <class CharType> void info      (const CharType* format, ...) const;
        template <class CharType> void info0     (const CharType* format, ...) const;
        template <class CharType> void info1     (const CharType* format, ...) const;
        template <class CharType> void info2     (const CharType* format, ...) const;
        template <class CharType> void info3     (const CharType* format, ...) const;
        template <class CharType> void operator()(const CharType* format, ...) const;

    private:
        typedef struct {
            ILogListener*       listener;
            LogLevel            level;
        } ListenerEntry;

        static ListenerEntry* s_listener;
        std::string  m_module_name;
        std::wstring m_module_name_w;

        void operator()(LogLevel level, const char*    format, va_list& list) const;
        void operator()(LogLevel level, const wchar_t* format, va_list& list) const;
    };

    template <class CharType> inline void Logger::error(const CharType* format, ...) const {
        va_list list; va_start(list, format); operator()(LogLevel::LOG_ERROR, format, list); va_end(list);
    }

    template <class CharType> inline void Logger::warning(const CharType* format, ...) const {
        va_list list; va_start(list, format); operator()(LogLevel::LOG_WARNING, format, list); va_end(list); 
    }

    template <class CharType> inline void Logger::info(const CharType* format, ...) const {
        va_list list; va_start(list, format); operator()(LogLevel::LOG_INFO_0, format, list); va_end(list);
    }

    template <class CharType> inline void Logger::info0(const CharType* format, ...) const {
        va_list list; va_start(list, format); operator()(LogLevel::LOG_INFO_0, format, list); va_end(list);
    }

    template <class CharType> inline void Logger::info1(const CharType* format, ...) const {
        va_list list; va_start(list, format); operator()(LogLevel::LOG_INFO_1, format, list); va_end(list);
    }

    template <class CharType> inline void Logger::info2(const CharType* format, ...) const {
        va_list list; va_start(list, format); operator()(LogLevel::LOG_INFO_2, format, list); va_end(list);
    }

    template <class CharType> inline void Logger::info3(const CharType* format, ...) const {
        va_list list; va_start(list, format); operator()(LogLevel::LOG_INFO_3, format, list); va_end(list);
    }

    template <class CharType> inline void Logger::operator()(const CharType* format, ...) const {
        va_list list; va_start(list, format); operator()(LogLevel::LOG_ANY, format, list); va_end(list);
    }

}   // namespace libspeedwire

#endif
