#pragma once

#include <exception>

#include "common.hpp"

#define ACL_EC_UNKNOWN 0
#define ACL_EC_SYMBOL_NOT_VISIBLE 1
#define ACL_EC_INVALID_MODIFIER 2
#define ACL_EC_STATIC_ACCESS_VIA_INSTANCE 3
#define ACL_EC_INSTANCE_ACCESS_VIA_STATIC 4
#define ACL_EC_GENERICS_MISMATCH 5
#define ACL_EC_TOO_MANY_GENERICS 6
#define ACL_EC_INSUFFICIENT_GENERICS 7

#define ASP_CORE_UNKNOWN "1.0.0"
#define ASP_INTEGER_LITERALS "1.0.1"
#define ASP_FLOATING_POINT_LITERALS "1.0.2"
#define ASP_STRING_ESCAPES "1.0.3"
#define ASP_COMMENT_BLOCKS "1.0.4"
#define ASP_SYMBOLS "1.0.5"
#define ASP_META_KEYWORDS "1.0.6"
#define ASP_GLOBAL_SCOPE "1.0.7"
#define ASP_LINE_TERMINATION "1.0.8"

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

void warn(const String& message);
}  // namespace acl