#include <iostream>
#include <unistd.h>

#include <glibmm.h>
#include <imgpack/logger.hh>

using ImgPack::Logger;

namespace {
    std::string level_to_string (Logger::Level level)
    {
        static std::string levels[] = {
            "<INFO>", "<WARNING>", "<ERROR>", "<FATAL>"
        };

        return levels[level];
    }
}

Logger::Logger () {}

Logger::~Logger () {}

void Logger::log (const std::string &context, Level level,
                  const std::string &message)
{
    std::cerr << "(" << Glib::get_prgname () << ":" << getpid () << ") "
              << level_to_string (level) << " "
              << message << " "
              << "(in " << context << ")" << std::endl;
}


using ImgPack::LogLine;

LogLine::LogLine (std::string context, Logger::Level level) :
    context (std::move (context)),
    level (level)
{}

LogLine::~LogLine ()
{
    Logger::instance ().log (context, level, buffer.str ());
}
