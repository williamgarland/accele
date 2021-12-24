#include "exceptions.hpp"

namespace acl {
    LexerException::LexerException(): message("Lexer exception") {}
    LexerException::LexerException(const SourceMeta& meta, const String& message) {
        StringBuffer sb;
        sb << "In module ";
        sb << meta.file << " at " << meta.line << ":" << meta.col << ": " << message;
        this->message = sb.str();
    }
    const char* LexerException::what() const noexcept {
        return message.c_str();
    }

    InvalidInputException::InvalidInputException(): message("Invalid input") {}
    InvalidInputException::InvalidInputException(const SourceMeta& meta, int input) {
        StringBuffer sb;
        sb << "Invalid input '" << (char) input << "' in module ";
        sb << meta.file << " at " << meta.line << ":" << meta.col;
        message = sb.str();
    }
    const char* InvalidInputException::what() const noexcept {
        return message.c_str();
    }
}