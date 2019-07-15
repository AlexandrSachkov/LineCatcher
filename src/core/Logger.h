/*
 * This file is part of the Line Catcher distribution (https://github.com/AlexandrSachkov/LineCatcher).
 * Copyright (c) 2019 Alexandr Sachkov.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

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