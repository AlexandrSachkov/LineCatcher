#pragma once

#include <string>
#include <regex>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <memory>

namespace PLP {
    class TextComparator {
    public:
        virtual bool initialize() = 0;
        virtual bool match(const char* data, unsigned int size) = 0;
    };

    class MatchMultiple : public TextComparator {
    public:
        MatchMultiple(const std::vector<std::shared_ptr<TextComparator>>& comparators) {
            _comparators = comparators;
        }

        bool initialize() override {
            for (auto& comparator : _comparators) {
                if (!comparator->initialize()) {
                    return false;
                }
            }
            return true;
        }

        bool match(const char* data, unsigned int size) override {
            for (auto& comparator : _comparators) {
                if (!comparator->match(data, size)) {
                    return false;
                }
            }
            return true;
        }

    private:
        std::vector<std::shared_ptr<TextComparator>> _comparators;
    };

    class Contains : public TextComparator {
    public:
        Contains(const std::string& text) : _text(text) {}

        bool initialize() override {
            return true;
        }

        bool match(const char* data, unsigned int size) override {
            return std::string::npos != std::string(data, size).find(_text);
        }

    private:
        std::string _text;
    };

    class RegexMatch : public TextComparator {
    public:
        RegexMatch(const std::string& regexPattern, bool ignoreCase) : _regexPattern(regexPattern), _ignoreCase(ignoreCase) {}

        bool initialize() override {
            std::regex_constants::syntax_option_type flags = std::regex_constants::optimize;
            if (_ignoreCase) {
                flags |= std::regex_constants::icase;
            }

            try {
                _regex = std::regex(_regexPattern, flags);
            } catch (std::regex_error&) {
                return false;
            }

            return true;
        }

        bool match(const char* data, unsigned int size) override {
            return std::regex_match(std::string(data, size), _regex);
        }

    private:
        std::string _regexPattern;
        bool _ignoreCase;
        std::regex _regex;
    };

    class Split : public TextComparator {
    public:
        Split(const std::string& splitText, const std::unordered_map<int, std::shared_ptr<TextComparator>>& sliceComparators) 
            : _splitText(splitText){
            try {
                _sliceComparators.resize(sliceComparators.size());

                for (auto& it : sliceComparators) {
                    _sliceComparators.push_back({ it.first, it.second });
                }

                std::sort(_sliceComparators.begin(), _sliceComparators.end(),
                    [](std::pair<int, std::shared_ptr<TextComparator>>& comp1, std::pair<int, std::shared_ptr<TextComparator>>& comp2) {
                    return comp1.first > comp2.first;
                });

                _sliceStartLength.reserve(100);
            } catch (std::bad_alloc&) {
                _internalFailure = true;
            }
        }

        bool initialize() override {
            if (_internalFailure) {
                return false;
            }

            for (auto& comp : _sliceComparators) {
                if (!comp.second->initialize()) {
                    return false;
                }
            }
            return true;
        }

        bool match(const char* data, unsigned int size) override {
            _sliceStartLength.clear();

            std::string sData(data, size);
            unsigned int pos = 0;
            unsigned int offset = 0;
            unsigned int length = 0;
            while ((pos = (unsigned int)sData.find(_splitText, offset)) != std::string::npos) {
                length = pos - offset + (unsigned int)_splitText.size();
                _sliceStartLength.emplace_back(offset, length);
                offset += (unsigned int)_splitText.size();
            }

            if (offset < _splitText.size() - 1) {
                _sliceStartLength.emplace_back(offset, (unsigned int)_splitText.size() - offset);
            }

            // quick check if one of the comparators is out of bounds
            for (auto& comparator : _sliceComparators) {
                if ((comparator.first >= 0 && comparator.first >= _sliceStartLength.size()) ||
                    (comparator.first < 0 && -comparator.first >= _sliceStartLength.size())) {
                    return false;
                }
            }

            int sliceIndex;
            for (auto& comparator : _sliceComparators) {
                sliceIndex = comparator.first;
                if (sliceIndex < 0) {
                    sliceIndex = (int)_sliceStartLength.size() - sliceIndex;
                }

                if (!comparator.second->match(data + _sliceStartLength[sliceIndex].first, _sliceStartLength[sliceIndex].second)) {
                    return false;
                }
            }

            return true;
        }

    private:
        bool _internalFailure = false;
        std::string _splitText;
        std::vector<std::pair<int, std::shared_ptr<TextComparator>>>  _sliceComparators;
        std::vector<std::pair<unsigned int, unsigned int>> _sliceStartLength;
    };
}