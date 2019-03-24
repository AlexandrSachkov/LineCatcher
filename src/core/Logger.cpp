#include "Logger.h"
#include "tbb/spin_mutex.h"

namespace PLP {

    std::map <std::string, std::function<void(LOG_LEVEL, const char*)>> Logger::_subscribers;
    std::unique_ptr<tbb::spin_mutex> Logger::_interfaceLock = std::unique_ptr<tbb::spin_mutex>(new tbb::spin_mutex());

    void Logger::subscribe(const std::string& name, std::function<void(LOG_LEVEL, const char*)> f) {
        tbb::spin_mutex::scoped_lock lock(*_interfaceLock);
        _subscribers.insert({name, f});
    }

    void Logger::unsubscribe(const std::string& name) {
        tbb::spin_mutex::scoped_lock lock(*_interfaceLock);

        auto it = _subscribers.find(name);
        if (it != _subscribers.end()) {
            _subscribers.erase(it);
        }
    }

    void Logger::send(LOG_LEVEL level, const std::string& msg) {
        tbb::spin_mutex::scoped_lock lock(*_interfaceLock);

        for (auto it : _subscribers) {
            it.second(level, msg.c_str());
        }
    }
}