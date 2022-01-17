#pragma once

#include <exception>
#include <iostream>

#include "common.hpp"

namespace acl {
namespace ec {
using ErrorCode = int;
constexpr ErrorCode UNKNOWN = 0;
constexpr ErrorCode SYMBOL_NOT_VISIBLE = 1;
constexpr ErrorCode INVALID_MODIFIER = 2;
constexpr ErrorCode STATIC_ACCESS_VIA_INSTANCE = 3;
constexpr ErrorCode INSTANCE_ACCESS_VIA_STATIC = 4;
constexpr ErrorCode GENERICS_MISMATCH = 5;
constexpr ErrorCode TOO_MANY_GENERICS = 6;
constexpr ErrorCode INSUFFICIENT_GENERICS = 7;
constexpr ErrorCode DUPLICATE_SYMBOL = 8;
constexpr ErrorCode DUPLICATE_IMPORT = 9;
constexpr ErrorCode DUPLICATE_IMPORT_ALIAS = 10;
constexpr ErrorCode ARGUMENT_TYPE_MISMATCH = 11;
constexpr ErrorCode TOO_MANY_ARGUMENTS = 12;
constexpr ErrorCode INSUFFICIENT_ARGUMENTS = 13;
constexpr ErrorCode STATIC_SELF = 14;
constexpr ErrorCode STATIC_SUPER = 15;
constexpr ErrorCode INVALID_COMMENT_BLOCK_END = 16;
constexpr ErrorCode INVALID_FLOAT_LITERAL = 17;
constexpr ErrorCode INVALID_HEX_LITERAL = 18;
constexpr ErrorCode INVALID_OCTAL_LITERAL = 19;
constexpr ErrorCode INVALID_BINARY_LITERAL = 20;
constexpr ErrorCode INVALID_LEXICAL_SYMBOL = 21;
constexpr ErrorCode INVALID_UNICODE_ESCAPE_SEQUENCE = 22;
constexpr ErrorCode INVALID_INTERPOLATION = 23;
constexpr ErrorCode INVALID_ESCAPE_SEQUENCE = 24;
constexpr ErrorCode INVALID_STRING_LITERAL_END = 25;
constexpr ErrorCode INVALID_INPUT = 26;
constexpr ErrorCode INVALID_TAG = 27;
constexpr ErrorCode INVALID_TOKEN = 28;
constexpr ErrorCode DUPLICATE_VARIABLE_BLOCK = 29;
constexpr ErrorCode NONSTATIC_TEMPLATE_VARIABLE = 30;
constexpr ErrorCode NONFRONTED_SOURCE_LOCK = 31;
constexpr ErrorCode DUPLICATE_DEFAULT_CASE = 32;
constexpr ErrorCode NONFINAL_VARIADIC_PARAMETER = 33;
constexpr ErrorCode INVALID_RETURN_STATEMENT = 34;
constexpr ErrorCode INVALID_THROW_STATEMENT = 35;
constexpr ErrorCode INVALID_FUNCTION_CALLER = 36;
constexpr ErrorCode UNDEFINED_SYMBOL = 37;
constexpr ErrorCode INVALID_SYMBOL_FOR_EXPRESSION = 38;
constexpr ErrorCode TEMPLATE_CONSTRUCTOR = 39;
constexpr ErrorCode UNRESOLVED_SYMBOL = 40;
constexpr ErrorCode UNRESOLVED_IMPORT = 41;
constexpr ErrorCode EC_LAST = UNRESOLVED_IMPORT;

enum class ErrorType { INFO, WARNING, ERROR };

ErrorType getErrorCodeType(ErrorCode ec);
const String& getErrorCodeTitle(ErrorCode ec);
const String& getErrorCodeId(ErrorCode ec);
}  // namespace ec

namespace log {
void warn(std::ostream& dest, const String& message,
		  const String& terminator = "\n");
void error(std::ostream& dest, const String& message,
		   const String& terminator = "\n");
}  // namespace log

struct Token;
enum class TokenType;
struct Symbol;
struct Import;

class Diagnoser {
   public:
	const CompilerContext& ctx;

   private:
	std::ostream& dest;

   public:
	Diagnoser(const CompilerContext& ctx, std::ostream& dest);
	void diagnose(ec::ErrorCode ec, const String& message);
	void diagnose(ec::ErrorCode ec);
	void diagnose(ec::ErrorCode ec, const SourceMeta& location,
				  int highlightLength);
	void diagnose(ec::ErrorCode ec, const SourceMeta& location,
				  int highlightLength, const String& message);

   public:
	void diagnoseMultiLineCommentEnd(const SourceMeta& location);
	void diagnoseFloatLiteral(const SourceMeta& location);
	void diagnoseHexLiteral(const SourceMeta& location);
	void diagnoseOctalLiteral(const SourceMeta& location);
	void diagnoseBinaryLiteral(const SourceMeta& location);
	void diagnoseSourceLock(const Token* token);
	void diagnoseInvalidToken(TokenType expected, const Token* received);
	void diagnoseInvalidToken(const String& expectedMessage,
							  const Token* received);
	void diagnoseInvalidTokenWithMessage(const String& message,
										 const Token* received);
	void diagnoseInvalidModifier(const Token* token);
	void diagnoseDuplicateSymbol(Symbol* original, Symbol* duplicate);
	void diagnoseDuplicateImport(Import* original, Import* duplicate);
	void diagnoseSymbolNotVisible(const SourceMeta& refMeta,
								  const Symbol* referent);
	void diagnoseStaticViaInstance(const SourceMeta& refMeta,
								   const Symbol* referent);
	void diagnoseInstanceViaStatic(const SourceMeta& refMeta,
								   const Symbol* referent);
};

struct AcceleException : public std::exception {
	ec::ErrorCode ec;
	const SourceMeta* sourceMeta;
	int highlightLength;
	String message;

	AcceleException(ec::ErrorCode ec, const SourceMeta& sourceMeta,
					int highlightLength, const String& message);
	AcceleException();
	virtual ~AcceleException();
};

class LexerPanicException : public AcceleException {};

class ParserPanicException : public AcceleException {};

struct Symbol;

struct DuplicateSymbolException : public AcceleException {
	Symbol* original;
	Symbol* duplicate;

	DuplicateSymbolException(Symbol* original, Symbol* duplicate);
	virtual ~DuplicateSymbolException();
};

struct Import;

struct DuplicateImportException : public AcceleException {
	Import* original;
	Import* duplicate;

	DuplicateImportException(Import* original, Import* duplicate);
	virtual ~DuplicateImportException();
};

struct UnresolvedSymbolException : public AcceleException {
	const Token* id;

	UnresolvedSymbolException(const Token* id);
	virtual ~UnresolvedSymbolException();
};

struct RecursiveResolutionException : public AcceleException {};
}  // namespace acl