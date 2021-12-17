#pragma once

#include <exception>

#include "common.hpp"

namespace acl {
    class LexerException: public std::exception {
        String message;
    public:
        LexerException();
        LexerException(const SourceMeta& meta, const String& message);
        virtual const char* what() const noexcept;
    };

    class InvalidInputException: public std::exception {
        String message;
    public:
        InvalidInputException();
        InvalidInputException(const SourceMeta& meta, int input);
        virtual const char* what() const noexcept;
    };
}