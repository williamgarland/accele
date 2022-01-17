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
	if (meta.moduleInfo) {
		sb << "In module ";
		sb << meta.moduleInfo->name << " at " << meta.line << ":" << meta.col
		   << ": ";
	}
	sb << message;
	this->message = sb.str();
}

LexerException::LexerException()
	: AclException(ASP_CORE_UNKNOWN, SourceMeta{nullptr, -1, -1, -1},
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

UnresolvedImportException::UnresolvedImportException(const String& protocol,
													 const SourceMeta& meta,
													 const String& message)
	: AclException(protocol, meta, message) {}

UnresolvedImportException::~UnresolvedImportException() {}

}  // namespace acl