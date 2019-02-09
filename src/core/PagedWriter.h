#pragma once

namespace PLP {
    class PagedWriter {
    public:
        virtual ~PagedWriter() {}
        virtual bool write(const char* data, unsigned long long size) = 0;
        virtual bool setPosition(unsigned long long fileOffset) = 0;
        virtual bool setPositionEnd() = 0;
        virtual bool flush() = 0;
    };
}