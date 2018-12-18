#pragma once

#include <string>
#include <memory>

#include "common/helper/singleton.h"
#include "spdlog/sinks/stdout_sinks.h"
#include "spdlog/spdlog.h"

namespace spd = spdlog;

class Logger {
public:
    Logger() : Logger("video") {

    }

    Logger(const std::string &moduleLibraryName) {
        //delegate = spd::rotating_logger_mt(moduleName, ".", 1048576 * 5, 3);
        delegate = spd::stdout_logger_mt(moduleLibraryName);

        delegate->flush_on(parseLevel());
        spd::set_level(parseLevel());
    }

    ~ Logger() {
        spd::drop(moduleName);
    }

    std::shared_ptr<spdlog::logger> getLogger() {
        return delegate;
    }

private:
    spd::level::level_enum parseLevel() {
        spd::level::level_enum level = spdlog::level::info;
        const char *levelEnv = getenv("SPD_LOG_LEVEL");
        if (nullptr != levelEnv && strlen(levelEnv) > 0) {
            level = spd::level::from_str(levelEnv);
        }
        return level;
    }

private:
    std::string moduleName;
    std::shared_ptr<spdlog::logger> delegate;
};

typedef Singleton<Logger> LoggerSingleton;


#define LOG_TRACE(...) LoggerSingleton::getInstance().getLogger()->trace(__VA_ARGS__)
#define LOG_DEBUG(...) LoggerSingleton::getInstance().getLogger()->debug(__VA_ARGS__)
#define LOG_ERROR(...) LoggerSingleton::getInstance().getLogger()->error(__VA_ARGS__)
#define LOG_WARN(...) LoggerSingleton::getInstance().getLogger()->warn(__VA_ARGS__)
#define LOG_INFO(...) LoggerSingleton::getInstance().getLogger()->info(__VA_ARGS__)
#define LOG_CRITICAL(...) LoggerSingleton::getInstance().getLogger()->critical(__VA_ARGS__)
