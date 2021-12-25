#pragma once

#include <exception>

#include "common.hpp"

namespace acl {
class LexerException : public std::exception {
	String message;

   public:
	LexerException();
	LexerException(const SourceMeta& meta, const String& message);
	virtual ~LexerException();
	virtual const char* what() const noexcept;
};

class InvalidInputException : public std::exception {
	String message;

   public:
	InvalidInputException();
	InvalidInputException(const SourceMeta& meta, int input);
	virtual ~InvalidInputException();
};

class ParserException : public std::exception {
   protected:
	String message;

   public:
	ParserException(const SourceMeta& meta,
					const String& message = "Parser exception");
	virtual ~ParserException();
	virtual const char* what() const noexcept;
};

enum class TokenType;
struct Token;

class TokenMismatchException : public ParserException {
   public:
	TokenMismatchException(TokenType expected, Token* received);
	virtual ~TokenMismatchException();
};
}  // namespace acl