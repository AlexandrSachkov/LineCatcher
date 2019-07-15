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

namespace PLP {
    class FileReaderI;
    class IndexWriterI {
    public:
        virtual ~IndexWriterI() {}
        virtual bool appendCurrLine(const FileReaderI* fReader) = 0;
        virtual bool appendCurrLine(unsigned long long lineNumber, unsigned long long fileOffset) = 0;
        virtual unsigned long long getNumResults() const = 0;
        virtual void release() = 0;
    };
}