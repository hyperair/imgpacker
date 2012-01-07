#ifndef IMGPACK_LOGGER_HH
#define IMGPACK_LOGGER_HH

#include <string>
#include <sstream>
#include <utility>

#include <nihpp/singleton.hh>

namespace ImgPack
{
    class Logger : public nihpp::Singleton<Logger>
    {
    private:
        friend class nihpp::Singleton<Logger>;

        Logger ();
        ~Logger ();

    public:
        enum Level {
            info = 0,
            warning,
            error,
            fatal,

            levels
        };

        void log (const std::string &context, Level level,
                  const std::string &message);
    };

    class LogLine
    {
    public:
        LogLine (std::string context, Logger::Level level);
        ~LogLine ();

        template <typename T>
        std::ostream &operator<< (T &&stuff)
        {
            return (buffer << std::forward<T> (stuff));
        }

    private:
        const std::string  context;
        Logger::Level      level;

        std::ostringstream buffer;
    };
}

#define _STR_(str) #str
#define _STR(str) _STR_(str)
#define LOG(level)                                              \
    ::ImgPack::LogLine (::std::string (__PRETTY_FUNCTION__) +   \
                        " [" __FILE__ ":" _STR(__LINE__) "]",   \
                        ::ImgPack::Logger::level)

#endif // IMGPACK_LOGGER_HH