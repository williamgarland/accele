#pragma once

#include <exception>

#include "common.hpp"

#define ASP_CORE_UNKNOWN "1.0.0"
#define ASP_INTEGER_LITERALS "1.0.1"
#define ASP_FLOATING_POINT_LITERALS "1.0.2"
#define ASP_STRING_ESCAPES "1.0.3"
#define ASP_COMMENT_BLOCKS "1.0.4"
#define ASP_SYMBOLS "1.0.5"
#define ASP_META_KEYWORDS "1.0.6"

#define ASP_SOURCE_LOCK_FRONTING "1.1.1"

namespace acl {
class AclException : public std::exception {
	String message;

   protected:
	void updateMessage(const String& protocol, const SourceMeta& meta,
					   const String& message);

   public:
	AclException();
	AclException(const String& protocol, const SourceMeta& meta,
				 const String& message);
	virtual ~AclException();
	virtual const char* what() const noexcept;
};

class LexerException : public AclException {
   public:
	LexerException();
	LexerException(const SourceMeta& meta, const String& message);
	virtual ~LexerException();
};

class InvalidInputException : public LexerException {
   public:
	InvalidInputException();
	InvalidInputException(const SourceMeta& meta, int input);
	virtual ~InvalidInputException();
};

class ParserException : public AclException {
   public:
	ParserException(const SourceMeta& meta,
					const String& message = "Parser exception");
	virtual ~ParserException();
};

enum class TokenType;
struct Token;

class TokenMismatchException : public ParserException {
   public:
	TokenMismatchException(TokenType expected, Token* received);
	virtual ~TokenMismatchException();
};
}  // namespace acl