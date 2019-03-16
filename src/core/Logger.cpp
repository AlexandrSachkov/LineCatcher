#include "Logger.h"

namespace PLP {

    std::map <std::string, std::function<void(LOG_LEVEL, const char*)>> Logger::_subscribers;

    void Logger::subscribe(const std::string& name, std::function<void(LOG_LEVEL, const char*)> f) {
        _subscribers.insert({name, f});
    }

    void Logger::unsubscribe(const std::string& name) {
        auto it = _subscribers.find(name);
        if (it != _subscribers.end()) {
            _subscribers.erase(it);
        }
    }

    void Logger::send(LOG_LEVEL level, const std::string& msg) {
        for (auto it : _subscribers) {
            it.second(level, msg.c_str());
        }
    }
}