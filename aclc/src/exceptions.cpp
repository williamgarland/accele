#include "exceptions.hpp"

#include "lexer.hpp"

namespace acl {
LexerException::LexerException() : message("Lexer exception") {}
LexerException::LexerException(const SourceMeta& meta, const String& message) {
	StringBuffer sb;
	sb << "In module ";
	sb << meta.file << " at " << meta.line << ":" << meta.col << ": "
	   << message;
	this->message = sb.str();
}
const char* LexerException::what() const noexcept { return message.c_str(); }

InvalidInputException::InvalidInputException() : message("Invalid input") {}
InvalidInputException::InvalidInputException(const SourceMeta& meta,
											 int input) {
	StringBuffer sb;
	sb << "Invalid input '" << (char)input << "' in module ";
	sb << meta.file << " at " << meta.line << ":" << meta.col;
	message = sb.str();
}

ParserException::ParserException(const SourceMeta& meta,
								 const String& message) {
	StringBuffer sb;
	sb << "In module ";
	sb << meta.file << " at " << meta.line << ":" << meta.col << ": "
	   << message;
	this->message = sb.str();
}

const char* ParserException::what() const noexcept { return message.c_str(); }

TokenMismatchException::TokenMismatchException(TokenType expected,
											   Token* received)
	: ParserException(received->meta, "") {
	StringBuffer sb;
	sb << "In module ";
	sb << received->meta.file << " at " << received->meta.line << ":"
	   << received->meta.col << ": ";
	sb << "Expected token with type " << (int)expected << ", received ["
	   << (int)received->type << "]: " << received->data;
	this->message = sb.str();
}

}  // namespace acl