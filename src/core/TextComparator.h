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

#include "Utils.h"

#include <string>
#include <regex>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <memory>
#include <cwctype>
#include <functional>

namespace PLP {
    class TextComparator {
    public:
        virtual bool initialize() = 0;
        virtual bool match(const char* data, unsigned int size) = 0;
        virtual bool match(const std::string& str) = 0;
    };

    class MatchAll : public TextComparator {
    public:
        MatchAll(const std::vector<std::shared_ptr<TextComparator>>& comparators) {
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

        bool match(const std::string& str) {
            return match(str.c_str(), (unsigned int)str.length());
        }

    private:
        std::vector<std::shared_ptr<TextComparator>> _comparators;
    };

    class MatchAny : public TextComparator {
    public:
        MatchAny(const std::vector<std::shared_ptr<TextComparator>>& comparators) {
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
                if (comparator->match(data, size)) {
                    return true;
                }
            }
            return false;
        }

        bool match(const std::string& str) {
            return match(str.c_str(), (unsigned int)str.length());
        }

    private:
        std::vector<std::shared_ptr<TextComparator>> _comparators;
    };

    class MatchNot : public TextComparator {
    public:
        MatchNot(std::shared_ptr<TextComparator> comparator) : _comparator(comparator) {}

        bool initialize() override {
            return _comparator->initialize();
        }

        bool match(const char* data, unsigned int size) override {
            return !_comparator->match(data, size);
        }

        bool match(const std::string& str) {
            return match(str.c_str(), (unsigned int)str.length());
        }

    private:
        std::shared_ptr<TextComparator> _comparator;
    };

    class MatchString : public TextComparator {
    public:
        MatchString(const std::string& text, bool exact) {
            std::wstring wText = string_to_wstring(text);
            if (exact) {
                _match = [text](const char* data, unsigned int size) {
                    return std::string(data, size).compare(text) == 0;
                };
            } else {
                _match = [text](const char* data, unsigned int size) {
                    return std::string::npos != std::string(data, size).find(text);
                };
            }
        }

        bool initialize() override {
            return true;
        }

        bool match(const char* data, unsigned int size) override {
            return _match(data, size);
        }

        bool match(const std::string& str) {
            return match(str.c_str(), (unsigned int)str.length());
        }

    private:
        std::function<bool(const char* data, unsigned int size)> _match;
    };

    class MatchRegex : public TextComparator {
    public:
        MatchRegex(const std::string& regexPattern) : _regexPattern(regexPattern) {}

        bool initialize() override {
            std::regex_constants::syntax_option_type flags = std::regex_constants::optimize;
            try {
                std::regex regex(_regexPattern, flags);
                _match = [regex](const std::string& data) {
                    return std::regex_search(data, regex);
                };
            } catch (std::regex_error&) {
                return false;
            }

            return true;
        }

        bool match(const char* data, unsigned int size) override {
            return _match(std::string(data, size));
        }

        bool match(const std::string& str) {
            return _match(str);
        }

    private:
        std::function<bool(const std::string& data)> _match;
        std::string _regexPattern;
    };

    class MatchSubstrings : public TextComparator {
    public:
        MatchSubstrings(const std::string& splitText, bool trimLine, const std::unordered_map<int, std::shared_ptr<TextComparator>>& sliceComparators)
            : _splitText(splitText), _trimLine(trimLine) {
            try {
                for (auto& it : sliceComparators) {
                    _sliceComparators.push_back(it);
                }

                std::sort(_sliceComparators.begin(), _sliceComparators.end(),
                    [](const std::pair<int, std::shared_ptr<TextComparator>>& comp1, const std::pair<int, std::shared_ptr<TextComparator>>& comp2) {
                    return comp1.first < comp2.first;
                });

                _substrings.reserve(100);
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

        bool match(const char* str, unsigned int size) override {
            _substrings.clear();

            char* adjustedStr = const_cast<char*>(str);
            unsigned int adjustedStrSize = size;
            if (_trimLine) {
                stringTrim(str, size, adjustedStr, adjustedStrSize);
            }
            std::string sData(adjustedStr, adjustedStrSize);

            size_t pos = 0;
            unsigned int offset = 0;
            unsigned int length = 0;
            while ((pos = sData.find(_splitText, offset)) != std::string::npos) {
                length = (unsigned int)pos - offset + (unsigned int)_splitText.size();
                _substrings.emplace_back(sData.data() + offset, length);
                offset += length;
            }

            if (offset < sData.size() - 1) {
                _substrings.emplace_back(sData.data() + offset, (unsigned int)sData.size() - offset);
            }else if (_substrings.size() == 0) {
                _substrings.emplace_back(sData.data(), 0);
            }

            // quick check if one of the comparators is out of bounds
            for (auto& comparator : _sliceComparators) {
                if ((comparator.first >= 0 && comparator.first >= _substrings.size()) ||
                    (comparator.first < 0 && -comparator.first >= _substrings.size())) {
                    return false;
                }
            }

            int sliceIndex;
            for (auto& comparator : _sliceComparators) {
                sliceIndex = comparator.first;
                if (sliceIndex < 0) {
                    sliceIndex = (int)_substrings.size() + sliceIndex;
                }

                if (!comparator.second->match(_substrings[sliceIndex].first, _substrings[sliceIndex].second)) {
                    return false;
                }
            }

            return true;
        }

        bool match(const std::string& str) {
            return match(str.c_str(), (unsigned int)str.length());
        }

    private:
        bool _internalFailure = false;
        std::string _splitText;
        bool _trimLine;
        std::vector<std::pair<int, std::shared_ptr<TextComparator>>>  _sliceComparators;
        std::vector<std::pair<const char*, unsigned int>> _substrings;
    };

    class MatchWords : public TextComparator {
    public:
        MatchWords(const std::unordered_map<int, std::shared_ptr<TextComparator>>& wordComparators) {
            try {
                for (auto& it : wordComparators) {
                    _wordComparators.push_back({ it.first, it.second });
                }

                std::sort(_wordComparators.begin(), _wordComparators.end(),
                    [](const std::pair<int, std::shared_ptr<TextComparator>>& comp1, const std::pair<int, std::shared_ptr<TextComparator>>& comp2) {
                    return comp1.first < comp2.first;
                });

                _words.reserve(100);
            } catch (std::bad_alloc&) {
                _internalFailure = true;
            }
        }

        bool initialize() override {
            if (_internalFailure) {
                return false;
            }

            for (auto& comp : _wordComparators) {
                if (!comp.second->initialize()) {
                    return false;
                }
            }
            return true;
        }

        bool match(const char* str, unsigned int size) override {
            splitIntoWords(str, size, _words);

            // quick check if one of the comparators is out of bounds
            for (auto& comparator : _wordComparators) {
                if ((comparator.first >= 0 && comparator.first >= _words.size()) ||
                    (comparator.first < 0 && -comparator.first >= _words.size())) {
                    return false;
                }
            }

            int wordIndex;
            for (auto& comparator : _wordComparators) {
                wordIndex = comparator.first;
                if (wordIndex < 0) {
                    wordIndex = (int)_words.size() + wordIndex;
                }

                if (!comparator.second->match(_words[wordIndex].first, _words[wordIndex].second)) {
                    return false;
                }
            }

            return true;
        }

        bool match(const std::string& str) {
            return match(str.c_str(), (unsigned int)str.length());
        }

    private:
        bool _internalFailure = false;
        std::vector<std::pair<int, std::shared_ptr<TextComparator>>> _wordComparators;
        std::vector<std::pair<const char*, unsigned int>> _words;
    };

    class MatchCustom : public TextComparator {
    public:
        MatchCustom(const std::function<bool(const std::string&)>& match) : _match(match) {}

        bool initialize() override {
            return true;
        }

        bool match(const char* data, unsigned int size) override {
            return _match(std::string(data, size));
        }

        bool match(const std::string& str) {
            return _match(str);
        }

    private:
        std::function<bool(const std::string&)> _match;
    };
}