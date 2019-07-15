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

#include <vector>
#include <string>

namespace LC {
    class GenFileTracker {
    public:
        static void addFile(const std::wstring& path);
        static std::vector<std::wstring> getFiles();
        static int size();
        static void clear();
    private:
        GenFileTracker() = default;
        GenFileTracker(const GenFileTracker&) = delete;
        GenFileTracker& operator=(const GenFileTracker&) = delete;

        static std::vector<std::wstring> _files;
    };
}