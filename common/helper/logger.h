#pragma once

#include <string>
#include <memory>
#include <iostream>

#include "spdlog/spdlog.h"
#include "common/helper/singleton.h"

namespace spd = spdlog;
using namespace std;

class Logger {
public:
    Logger() : Logger("ALGO") {

    }

    Logger(const string &moduleLibraryName) {
        //delegate = spd::rotating_logger_mt(moduleName, ".", 1048576 * 5, 3);
        delegate = spd::stdout_logger_mt(moduleLibraryName);

        delegate->flush_on(spdlog::level::info);
        spd::set_level(spd::level::info);
    }

    ~ Logger() {
        spd::drop(moduleName);
    }

    shared_ptr<spdlog::logger> getLogger() {
        return delegate;
    }

private:
    string moduleName;
    shared_ptr<spdlog::logger> delegate;
};

typedef Singleton<Logger> LoggerSingleton;

//
// #ifndef suffix
// #define suffix()  std::string(msg).append("  //")\
//     .append(__FILENAME__).append(":").append(__func__)\
//     .append("()#").append(std::to_string(__LINE__))\
//     .append(".").c_str()
// #endif

#define LOG_TRACE(...) LoggerSingleton::getInstance().getLogger()->trace(__VA_ARGS__)
#define LOG_DEBUG(...) LoggerSingleton::getInstance().getLogger()->debug(__VA_ARGS__)
#define LOG_ERROR(...) LoggerSingleton::getInstance().getLogger()->error(__VA_ARGS__)
#define LOG_WARN(...) LoggerSingleton::getInstance().getLogger()->warn(__VA_ARGS__)
#define LOG_INFO(...) LoggerSingleton::getInstance().getLogger()->info(__VA_ARGS__)
#define LOG_CRITICAL(...) LoggerSingleton::getInstance().getLogger()->critical(__VA_ARGS__)
