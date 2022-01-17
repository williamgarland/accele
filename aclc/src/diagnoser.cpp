#include "diagnoser.hpp"

#include <iostream>

#include "ast.hpp"
#include "lexer.hpp"

namespace {
using namespace acl::ec;

struct ECData {
	ErrorType type;
	acl::String title;
	acl::String id;
};

const acl::Map<ErrorCode, ECData> ERROR_CODE_DATA = {
	{UNKNOWN, {ErrorType::ERROR, "Unknown error", "unknown"}},
	{SYMBOL_NOT_VISIBLE,
	 {ErrorType::ERROR, "Symbol not visible", "symbolNotVisible"}},
	{INVALID_MODIFIER,
	 {ErrorType::ERROR, "Invalid modifier", "invalidModifier"}},
	{STATIC_ACCESS_VIA_INSTANCE,
	 {ErrorType::WARNING, "Static access via instance", "staticViaInstance"}},
	{INSTANCE_ACCESS_VIA_STATIC,
	 {ErrorType::ERROR, "Instance access via static", "instanceViaStatic"}},
	{GENERICS_MISMATCH,
	 {ErrorType::ERROR, "Generics mismatch", "genericsMismatch"}},
	{TOO_MANY_GENERICS,
	 {ErrorType::ERROR, "Too many generics", "tooManyGenerics"}},
	{INSUFFICIENT_GENERICS,
	 {ErrorType::ERROR, "Insufficient generics", "tooFewGenerics"}},
	{DUPLICATE_SYMBOL,
	 {ErrorType::ERROR, "Duplicate symbol", "duplicateSymbol"}},
	{DUPLICATE_IMPORT,
	 {ErrorType::ERROR, "Duplicate import", "duplicateImport"}},
	{DUPLICATE_IMPORT_ALIAS,
	 {ErrorType::ERROR, "Duplicate import alias", "duplicateImportAlias"}},
	{ARGUMENT_TYPE_MISMATCH,
	 {ErrorType::ERROR, "Argument type mismatch", "argumentMismatch"}},
	{TOO_MANY_ARGUMENTS,
	 {ErrorType::ERROR, "Too many arguments", "tooManyArguments"}},
	{INSUFFICIENT_ARGUMENTS,
	 {ErrorType::ERROR, "Insufficient arguments", "tooFewArguments"}},
	{STATIC_SELF, {ErrorType::ERROR, "Static self", "staticSelf"}},
	{STATIC_SUPER, {ErrorType::ERROR, "Static super", "staticSuper"}},
	{INVALID_COMMENT_BLOCK_END,
	 {ErrorType::ERROR, "Invalid comment block end", "commentBlockEnd"}},
	{INVALID_FLOAT_LITERAL,
	 {ErrorType::ERROR, "Invalid float literal", "floatLiteral"}},
	{INVALID_HEX_LITERAL,
	 {ErrorType::ERROR, "Invalid hex literal", "hexLiteral"}},
	{INVALID_OCTAL_LITERAL,
	 {ErrorType::ERROR, "Invalid octal literal", "octalLiteral"}},
	{INVALID_BINARY_LITERAL,
	 {ErrorType::ERROR, "Invalid binary literal", "binaryLiteral"}},
	{INVALID_LEXICAL_SYMBOL,
	 {ErrorType::ERROR, "Invalid lexical symbol", "invalidLexicalSymbol"}},
	{INVALID_UNICODE_ESCAPE_SEQUENCE,
	 {ErrorType::ERROR, "Invalid unicode escape sequence",
	  "invalidUnicodeEscape"}},
	{INVALID_INTERPOLATION,
	 {ErrorType::ERROR, "Invalid interpolation escape sequence",
	  "invalidInterpolation"}},
	{INVALID_ESCAPE_SEQUENCE,
	 {ErrorType::ERROR, "Invalid escape sequence", "invalidEscape"}},
	{INVALID_STRING_LITERAL_END,
	 {ErrorType::ERROR, "Invalid string literal end", "stringLiteralEnd"}},
	{INVALID_INPUT, {ErrorType::ERROR, "Invalid input", "invalidInput"}},
	{INVALID_TAG, {ErrorType::ERROR, "Invalid tag", "invalidTag"}},
	{INVALID_TOKEN, {ErrorType::ERROR, "Invalid token", "invalidToken"}},
	{DUPLICATE_VARIABLE_BLOCK,
	 {ErrorType::ERROR, "Duplicate variable block", "duplicateVarBlock"}},
	{NONSTATIC_TEMPLATE_VARIABLE,
	 {ErrorType::ERROR, "Non-static template variable",
	  "nonStaticTemplateVar"}},
	{NONFRONTED_SOURCE_LOCK,
	 {ErrorType::WARNING, "Non-fronted source lock tag", "sourceLockFronting"}},
	{DUPLICATE_DEFAULT_CASE,
	 {ErrorType::ERROR, "Duplicate default switch case",
	  "duplicateDefaultCase"}},
	{NONFINAL_VARIADIC_PARAMETER,
	 {ErrorType::ERROR, "Non-final variadic parameter", "nonFinalVarargs"}},
	{INVALID_RETURN_STATEMENT,
	 {ErrorType::ERROR, "Invalid return statement", "invalidReturn"}},
	{INVALID_THROW_STATEMENT,
	 {ErrorType::ERROR, "Invalid throw statement", "invalidThrow"}},
	{INVALID_FUNCTION_CALLER,
	 {ErrorType::ERROR, "Invalid function caller", "invalidFunctionCaller"}},
	{UNDEFINED_SYMBOL,
	 {ErrorType::ERROR, "Undefined symbol", "undefinedSymbol"}},
	{INVALID_SYMBOL_FOR_EXPRESSION,
	 {ErrorType::ERROR, "Invalid symbol for expression", "invalidSymbolExpr"}},
	{TEMPLATE_CONSTRUCTOR,
	 {ErrorType::ERROR, "Template constructor", "templateConstructor"}},
	 {UNRESOLVED_SYMBOL, {ErrorType::ERROR, "Unresolved symbol", "unresolvedSymbol"}},
	 {UNRESOLVED_IMPORT, {ErrorType::ERROR, "Unresolved import", "unresolvedImport"}}};
}  // namespace

namespace acl {
namespace log {
void warn(std::ostream& dest, const String& message, const String& terminator) {
	dest << "\u001b[33m[WARN]\u001b[0m: " << message << terminator;
}

void error(std::ostream& dest, const String& message,
		   const String& terminator) {
	dest << "\u001b[31m[ERROR]\u001b[0m: " << message << terminator;
}
}  // namespace log
}  // namespace acl

namespace {
using namespace acl;

void printCodeSnippet(std::ostream& dest, const CompilerContext& ctx,
					  const SourceMeta& highlightBegin, int highlightLength,
					  ec::ErrorType highlightType);

String getLocationInfo(const SourceMeta& location);
}  // namespace

namespace acl {
namespace ec {
ErrorType getErrorCodeType(ErrorCode ec) { return ERROR_CODE_DATA.at(ec).type; }

const String& getErrorCodeTitle(ErrorCode ec) {
	return ERROR_CODE_DATA.at(ec).title;
}
}  // namespace ec

Diagnoser::Diagnoser(const CompilerContext& ctx, std::ostream& dest)
	: ctx(ctx), dest(dest) {}

void Diagnoser::diagnose(ec::ErrorCode ec, const String& message) {
	const auto& title = ec::getErrorCodeTitle(ec);

	StringBuffer sb;
	sb << title;

	sb << " (ACL" << std::setfill('0') << std::setw(4) << ec << ")";

	if (!message.empty()) sb << " - " << message;

	std::cout << sb.str() << "\n";
}

void Diagnoser::diagnose(ec::ErrorCode ec) { diagnose(ec, ""); }

void Diagnoser::diagnose(ec::ErrorCode ec, const SourceMeta& location,
						 int highlightLength) {
	log::error(dest, getLocationInfo(location));
	diagnose(ec);
	printCodeSnippet(dest, ctx, location, highlightLength,
					 getErrorCodeType(ec));
}

void Diagnoser::diagnose(ec::ErrorCode ec, const SourceMeta& location,
						 int highlightLength, const String& message) {
	log::error(dest, getLocationInfo(location));
	diagnose(ec, message);
	printCodeSnippet(dest, ctx, location, highlightLength,
					 getErrorCodeType(ec));
}

void Diagnoser::diagnoseMultiLineCommentEnd(const SourceMeta& location) {
	log::error(dest, getLocationInfo(location));
	diagnose(ec::INVALID_COMMENT_BLOCK_END,
			 "The following comment block does not terminate:");
	printCodeSnippet(dest, ctx, location, 2, ec::ErrorType::ERROR);
}

void Diagnoser::diagnoseFloatLiteral(const SourceMeta& location) {
	log::error(dest, getLocationInfo(location));
	diagnose(ec::INVALID_FLOAT_LITERAL,
			 "Expected digits following the exponent marker");
	printCodeSnippet(dest, ctx, location, 1, ec::ErrorType::ERROR);
}

void Diagnoser::diagnoseHexLiteral(const SourceMeta& location) {
	log::error(dest, getLocationInfo(location));
	diagnose(ec::INVALID_HEX_LITERAL,
			 "Expected hexadecimal digits following the hex literal indicator");
	printCodeSnippet(dest, ctx, location, 1, ec::ErrorType::ERROR);
}

void Diagnoser::diagnoseOctalLiteral(const SourceMeta& location) {
	log::error(dest, getLocationInfo(location));
	diagnose(ec::INVALID_OCTAL_LITERAL,
			 "Expected octal digits following the octal literal indicator");
	printCodeSnippet(dest, ctx, location, 1, ec::ErrorType::ERROR);
}

void Diagnoser::diagnoseBinaryLiteral(const SourceMeta& location) {
	log::error(dest, getLocationInfo(location));
	diagnose(ec::INVALID_BINARY_LITERAL,
			 "Expected binary digits following the binary literal indicator");
	printCodeSnippet(dest, ctx, location, 1, ec::ErrorType::ERROR);
}

void Diagnoser::diagnoseSourceLock(const Token* token) {
	log::warn(dest, getLocationInfo(token->meta));
	diagnose(ec::NONFRONTED_SOURCE_LOCK,
			 "Source lock tags should be placed at the top of the module");
	printCodeSnippet(dest, ctx, token->meta, token->data.length() + 1,
					 ec::ErrorType::WARNING);  // +1 for the initial '@'
}

void Diagnoser::diagnoseInvalidToken(TokenType expected,
									 const Token* received) {
	diagnoseInvalidToken(getStringForTokenType(expected), received);
}

void Diagnoser::diagnoseInvalidToken(const String& expectedMessage,
									 const Token* received) {
	StringBuffer sb;
	sb << "Expected " << expectedMessage << ", but received " << received->data;
	diagnoseInvalidTokenWithMessage(sb.str(), received);
}

void Diagnoser::diagnoseInvalidTokenWithMessage(const String& message,
												const Token* received) {
	log::error(dest, getLocationInfo(received->meta));
	diagnose(ec::INVALID_TOKEN, message);
	printCodeSnippet(dest, ctx, received->meta, received->data.length(),
					 ec::ErrorType::ERROR);
}

void Diagnoser::diagnoseInvalidModifier(const Token* token) {
	log::error(dest, getLocationInfo(token->meta));
	StringBuffer sb;
	sb << token->data << " is not a modifier";
	diagnose(ec::INVALID_MODIFIER, sb.str());
	printCodeSnippet(dest, ctx, token->meta, token->data.length(),
					 ec::ErrorType::ERROR);
}

void Diagnoser::diagnoseDuplicateSymbol(Symbol* original, Symbol* duplicate) {
	log::error(dest, getLocationInfo(duplicate->sourceMeta));
	diagnose(ec::DUPLICATE_SYMBOL);
	std::cout << "Original declaration:\n";
	printCodeSnippet(dest, ctx, original->id->meta, original->id->data.length(),
					 ec::ErrorType::INFO);
	std::cout << "\nDuplicate declaration:\n";
	printCodeSnippet(dest, ctx, duplicate->id->meta,
					 duplicate->id->data.length(), ec::ErrorType::ERROR);
}

void Diagnoser::diagnoseDuplicateImport(Import* original, Import* duplicate) {
	log::error(dest, getLocationInfo(duplicate->sourceMeta));
	diagnose(ec::DUPLICATE_IMPORT);
	std::cout << "Original declaration:\n";
	printCodeSnippet(dest, ctx, original->id->meta, original->id->data.length(),
					 ec::ErrorType::INFO);
	std::cout << "\nDuplicate declaration:\n";
	printCodeSnippet(dest, ctx, duplicate->id->meta,
					 duplicate->id->data.length(), ec::ErrorType::ERROR);
}

void Diagnoser::diagnoseSymbolNotVisible(const SourceMeta& refMeta,
										 const Symbol* referent) {
	log::error(dest, getLocationInfo(refMeta));
	diagnose(ec::SYMBOL_NOT_VISIBLE);
	printCodeSnippet(dest, ctx, refMeta, referent->id->data.length(),
					 ec::ErrorType::ERROR);
}

void Diagnoser::diagnoseStaticViaInstance(const SourceMeta& refMeta,
										  const Symbol* referent) {
	log::error(dest, getLocationInfo(refMeta));
	diagnose(ec::STATIC_ACCESS_VIA_INSTANCE,
			 "Static members should be accessed in a static way");
	printCodeSnippet(dest, ctx, refMeta, referent->id->data.length(),
					 ec::ErrorType::WARNING);
}

void Diagnoser::diagnoseInstanceViaStatic(const SourceMeta& refMeta,
										  const Symbol* referent) {
	log::error(dest, getLocationInfo(refMeta));
	diagnose(ec::INSTANCE_ACCESS_VIA_STATIC,
			 "Instance members cannot be accessed in a static way");
	printCodeSnippet(dest, ctx, refMeta, referent->id->data.length(),
					 ec::ErrorType::ERROR);
}

AcceleException::AcceleException(ec::ErrorCode ec, const SourceMeta& sourceMeta,
								 int highlightLength, const String& message)
	: ec(ec),
	  sourceMeta(&sourceMeta),
	  highlightLength(highlightLength),
	  message(message) {}

AcceleException::AcceleException()
	: ec(ec::UNKNOWN), sourceMeta(nullptr), highlightLength(0), message("") {}

AcceleException::~AcceleException() {}

DuplicateSymbolException::DuplicateSymbolException(Symbol* original,
												   Symbol* duplicate)
	: AcceleException(ec::DUPLICATE_SYMBOL, duplicate->sourceMeta,
					  duplicate->id->data.length(), ""),
	  original(original),
	  duplicate(duplicate) {}

DuplicateSymbolException::~DuplicateSymbolException() {}

DuplicateImportException::DuplicateImportException(Import* original,
												   Import* duplicate)
	: AcceleException(ec::DUPLICATE_IMPORT, duplicate->sourceMeta,
					  duplicate->id->data.length(), ""),
	  original(original),
	  duplicate(duplicate) {}

DuplicateImportException::~DuplicateImportException() {}

UnresolvedSymbolException::UnresolvedSymbolException(const Token* id)
	: id(id) {}

UnresolvedSymbolException::~UnresolvedSymbolException() {}
}  // namespace acl

namespace {
using namespace acl;

String getLocationInfo(const SourceMeta& location) {
	StringBuffer sb;
	sb << "In module \"" << location.moduleInfo->name << "\" at "
	   << location.line << ":" << location.col << " ("
	   << location.moduleInfo->path << ")";
	String str = sb.str();
	return str;
}

const Module* getModule(const CompilerContext& ctx,
						const ModuleInfo* moduleInfo) {
	for (const auto& m : ctx.modules) {
		if (&m->moduleInfo == moduleInfo) return m;
	}

	return nullptr;
}

String getTextColorForErrorType(ec::ErrorType type) {
	if (type == ec::ErrorType::INFO) return "\u001b[36m";
	if (type == ec::ErrorType::WARNING) return "\u001b[33m";
	if (type == ec::ErrorType::ERROR) return "\u001b[31m";
	return "\u001b[0m";
}

void printCodeSnippet(std::ostream& dest, const CompilerContext& ctx,
					  const SourceMeta& begin, int highlightLength,
					  ec::ErrorType highlightType) {
	auto highlightColor = getTextColorForErrorType(highlightType);

	auto m = getModule(ctx, begin.moduleInfo);

	if (!m) return;

	int maxNumberLength = 0;
	{
		StringBuffer sb;
		sb << begin.line + 1;
		String str = sb.str();
		maxNumberLength = str.length();
	}

	// print line above (if available)
	if (begin.line > 1) {
		dest << std::setfill('0') << std::setw(maxNumberLength)
			 << begin.line - 1 << " | ";
		dest << m->source[begin.line - 2]
			 << "\n";  // 1 for line before and then 1 more for the index
					   // of the line
	}

	// print problem line
	dest << std::setfill('0') << std::setw(maxNumberLength) << begin.line
		 << " | ";
	dest << m->source[begin.line - 1] << "\n";

	// print highlight
	for (int i = 0; i < maxNumberLength; i++)
		dest << " ";  // Account for line number
	dest << " | ";
	for (int i = 1; i <= m->source[begin.line - 1].length(); i++) {
		if (i >= begin.col && i <= begin.col + highlightLength - 1) {
			dest << highlightColor << "^\u001b[0m";
		} else
			dest << " ";
	}
	dest << "\n";

	// print line below
	if (begin.line < m->source.size()) {
		dest << std::setfill('0') << std::setw(maxNumberLength)
			 << begin.line + 1 << " | ";
		dest << m->source[begin.line] << "\n";
	}
}
}  // namespace