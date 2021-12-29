#include "exceptions.hpp"

#include <iostream>

#include "lexer.hpp"

namespace acl {
AclException::AclException() : message("Unknown error") {}
AclException::AclException(const String& protocol, const SourceMeta& meta,
						   const String& message) {
	updateMessage(protocol, meta, message);
}

AclException::~AclException() {}

const char* AclException::what() const noexcept { return message.c_str(); }

void AclException::updateMessage(const String& protocol, const SourceMeta& meta,
								 const String& message) {
	StringBuffer sb;
	sb << "The following violates ASP " << protocol << ":\n";
	sb << "In module ";
	sb << meta.file << " at " << meta.line << ":" << meta.col << ": "
	   << message;
	this->message = sb.str();
}

LexerException::LexerException()
	: AclException(ASP_CORE_UNKNOWN, SourceMeta{"(ERR)", -1, -1},
				   "Lexer exception") {}

LexerException::LexerException(const SourceMeta& meta, const String& message)
	: AclException(ASP_CORE_UNKNOWN, meta, message) {}

LexerException::~LexerException() {}

InvalidInputException::InvalidInputException() : LexerException() {}
InvalidInputException::InvalidInputException(const SourceMeta& meta, int input)
	: LexerException(meta, "Invalid input") {}

InvalidInputException::~InvalidInputException() {}

ParserException::ParserException(const SourceMeta& meta, const String& message)
	: AclException(ASP_CORE_UNKNOWN, meta, message) {}

ParserException::~ParserException() {}

TokenMismatchException::TokenMismatchException(TokenType expected,
											   Token* received)
	: ParserException(received->meta, "") {
	StringBuffer sb;
	sb << "Expected token with type " << (int)expected << ", received ["
	   << (int)received->type << "]: " << received->data;
	String newMessage = sb.str();
	updateMessage(ASP_CORE_UNKNOWN, received->meta, newMessage);
}

TokenMismatchException::~TokenMismatchException() {}

void warn(const String& message) { std::cout << "[WARN]: " << message << "\n"; }

}  // namespace acl