#pragma once

#include <string>
#include <functional>
#include <map>
#include <memory>

namespace tbb {
    class spin_mutex;
}

namespace PLP {
    enum LOG_LEVEL {
        INFO = 0,
        WARN = 1,
        ERR = 2
    };

    class Logger {
    public:
        static void subscribe(const std::string& name, std::function<void(LOG_LEVEL, const char*)> f);
        static void unsubscribe(const std::string& name);

        static void send(LOG_LEVEL level, const std::string& msg);
    private:
        static std::map <std::string, std::function<void(LOG_LEVEL, const char*)>> _subscribers;
        static std::unique_ptr<tbb::spin_mutex> _interfaceLock;
    };
}