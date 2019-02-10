#pragma once

#include <string>

namespace PLP {
    class FileWriterI {
    public:
        virtual ~FileWriterI() {}
        virtual bool append(const std::string& data) = 0;
        virtual bool append(const char* data, unsigned long long size) = 0;
        virtual bool appendLine(const std::string& line) = 0;
        virtual bool appendLine(const char* data, unsigned long long size) = 0;
        virtual void release() = 0;
    };
}
