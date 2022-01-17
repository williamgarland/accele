#include "lexer.hpp"

#include "diagnoser.hpp"
#include "exceptions.hpp"

namespace {
using namespace acl;
Map<TokenType, String> TOKEN_TYPE_STRINGS = {
	{TokenType::PUBLIC, "public"},
	{TokenType::PRIVATE, "private"},
	{TokenType::PROTECTED, "protected"},
	{TokenType::INTERNAL, "internal"},
	{TokenType::STATIC, "static"},
	{TokenType::UNSAFE, "unsafe"},
	{TokenType::ATOMIC, "atomic"},
	{TokenType::REF, "ref"},
	{TokenType::STRONG, "strong"},
	{TokenType::WEAK, "weak"},
	{TokenType::GREEDY, "greedy"},
	{TokenType::FINAL, "final"},
	{TokenType::OVERRIDE, "override"},
	{TokenType::INFIX, "infix"},
	{TokenType::PREFIX, "prefix"},
	{TokenType::POSTFIX, "postfix"},
	{TokenType::CLASS, "class"},
	{TokenType::STRUCT, "struct"},
	{TokenType::TEMPLATE, "template"},
	{TokenType::ENUM, "enum"},
	{TokenType::NAMESPACE, "namespace"},
	{TokenType::VAR, "var"},
	{TokenType::CONST, "const"},
	{TokenType::FUN, "fun"},
	{TokenType::SELF, "self"},
	{TokenType::SUPER, "super"},
	{TokenType::IF, "if"},
	{TokenType::ELIF, "elif"},
	{TokenType::ELSE, "else"},
	{TokenType::FOR, "for"},
	{TokenType::IN, "in"},
	{TokenType::WHILE, "while"},
	{TokenType::REPEAT, "repeat"},
	{TokenType::SWITCH, "switch"},
	{TokenType::CASE, "case"},
	{TokenType::DEFAULT, "default"},
	{TokenType::BREAK, "break"},
	{TokenType::CONTINUE, "continue"},
	{TokenType::RETURN, "return"},
	{TokenType::THROW, "throw"},
	{TokenType::THROWING, "throwing"},
	{TokenType::NOEXCEPT, "noexcept"},
	{TokenType::IMPORT, "import"},
	{TokenType::FROM, "from"},
	{TokenType::AS, "as"},
	{TokenType::ASYNC, "async"},
	{TokenType::AWAIT, "await"},
	{TokenType::RELEASE, "release"},
	{TokenType::GET, "get"},
	{TokenType::SET, "set"},
	{TokenType::INIT, "init"},
	{TokenType::CONSTRUCT, "construct"},
	{TokenType::DESTRUCT, "destruct"},
	{TokenType::TRY, "try"},
	{TokenType::CATCH, "catch"},
	{TokenType::AND, "and"},
	{TokenType::OR, "or"},
	{TokenType::NOT, "not"},
	{TokenType::AS_OPTIONAL, "as?"},
	{TokenType::AS_UNWRAPPED, "as!"},
	{TokenType::TRY_OPTIONAL, "try?"},
	{TokenType::TRY_UNWRAPPED, "try!"},
	{TokenType::ALIAS, "alias"},
	{TokenType::EXTERN, "extern"},
	{TokenType::FALL, "fall"},
	{TokenType::GLOBAL, "global"},
	{TokenType::IS, "is"},
	{TokenType::USES, "uses"},
	{TokenType::TILDE, "~"},
	{TokenType::EXCLAMATION_POINT, "!"},
	{TokenType::PERCENT, "%"},
	{TokenType::CARET, "^"},
	{TokenType::AMPERSAND, "&"},
	{TokenType::ASTERISK, "*"},
	{TokenType::LPAREN, "("},
	{TokenType::RPAREN, ")"},
	{TokenType::MINUS, "-"},
	{TokenType::EQUALS, "="},
	{TokenType::PLUS, "+"},
	{TokenType::LBRACKET, "["},
	{TokenType::RBRACKET, "]"},
	{TokenType::LBRACE, "{"},
	{TokenType::RBRACE, "}"},
	{TokenType::PIPE, "|"},
	{TokenType::COLON, ":"},
	{TokenType::LT, "<"},
	{TokenType::GT, ">"},
	{TokenType::COMMA, ","},
	{TokenType::DOT, "."},
	{TokenType::SLASH, "/"},
	{TokenType::QUESTION_MARK, "?"},
	{TokenType::DOUBLE_EQUALS, "=="},
	{TokenType::TRIPLE_EQUALS, "==="},
	{TokenType::TILDE_EQUALS, "~="},
	{TokenType::EXCLAMATION_POINT_EQUALS, "!="},
	{TokenType::EXCLAMATION_POINT_DOUBLE_EQUALS, "!=="},
	{TokenType::PERCENT_EQUALS, "%="},
	{TokenType::CARET_EQUALS, "^="},
	{TokenType::AMPERSAND_EQUALS, "&="},
	{TokenType::MINUS_EQUALS, "-="},
	{TokenType::PLUS_EQUALS, "+="},
	{TokenType::PIPE_EQUALS, "|="},
	{TokenType::LT_EQUALS, "<="},
	{TokenType::GT_EQUALS, ">="},
	{TokenType::SLASH_EQUALS, "/="},
	{TokenType::DOUBLE_AMPERSAND, "&&"},
	{TokenType::DOUBLE_PIPE, "||"},
	{TokenType::DOUBLE_ASTERISK, "**"},
	{TokenType::DOUBLE_MINUS, "--"},
	{TokenType::DOUBLE_PLUS, "++"},
	{TokenType::DOUBLE_LT, "<<"},
	{TokenType::DOUBLE_GT, ">>"},
	{TokenType::DOUBLE_DOT, ".."},
	{TokenType::TRIPLE_DOT, "..."},
	{TokenType::DOUBLE_QUESTION_MARK, "??"},
	{TokenType::QUESTION_MARK_DOT, "?."},
	{TokenType::MINUS_ARROW, "->"},
	{TokenType::EQUALS_ARROW, "=>"},
	{TokenType::SEMICOLON, ";"},
	{TokenType::COMPARE, "<=>"},
	{TokenType::DOUBLE_ASTERISK_EQUALS, "**="},
	{TokenType::DOUBLE_LT_EQUALS, "<<="},
	{TokenType::DOUBLE_GT_EQUALS, ">>="},
	{TokenType::META_NORETURN, "@noreturn"},
	{TokenType::META_STACKALLOC, "@stackalloc"},
	{TokenType::META_SRCLOCK, "@srclock"},
	{TokenType::META_LAXTHROW, "@laxthrow"},
	{TokenType::META_EXTERNALINIT, "@externalinit"},
	{TokenType::META_DEPRECATED, "@deprecated"},
	{TokenType::META_ENABLEWARNING, "@enablewarning"},
	{TokenType::META_DISABLEWARNING, "@disablewarning"},
	{TokenType::META_NOBUILTINS, "@nobuiltins"},
	{TokenType::ID, "identifier"},
	{TokenType::BOOLEAN_LITERAL, "a boolean literal"},
	{TokenType::INTEGER_LITERAL, "an integer literal"},
	{TokenType::HEX_LITERAL, "a hexadecimal literal"},
	{TokenType::OCTAL_LITERAL, "an octal literal"},
	{TokenType::BINARY_LITERAL, "a binary literal"},
	{TokenType::FLOAT_LITERAL, "a floating-point literal"},
	{TokenType::STRING_LITERAL, "a string literal"},
	{TokenType::NIL_LITERAL, "nil"},
	{TokenType::EOF_TOKEN, "end of file"},
	{TokenType::NL, "end of line"}};
}  // namespace

namespace acl {
Token::Token(TokenType type, const String& data, const SourceMeta& meta)
	: type(type), data(data), meta(meta) {}
Token::~Token() {}

StringToken::StringToken(TokenType type, const String& data,
						 const SourceMeta& meta,
						 const Map<int, String>& interpolations)
	: Token(type, data, meta), interpolations(interpolations) {}
StringToken::~StringToken() {}

Lexer::Lexer(const CompilerContext& ctx, const ModuleInfo& moduleInfo,
			 StringBuffer& buf)
	: moduleInfo(moduleInfo),
	  buf(buf),
	  line(1),
	  col(1),
	  diagnoser(ctx, std::cout),
	  currentPos(0) {}

SourceMeta Lexer::getSourceMeta() {
	return {&moduleInfo, currentPos, line, col};
}

int Lexer::get() { return buf.peek(); }

int Lexer::advance() {
	col++;
	currentPos++;
	return buf.get();
}

void Lexer::retract(char c) {
	col--;
	currentPos--;
	buf.putback(c);
}

[[noreturn]] void Lexer::panic() {
	while (hasNext() && !listContains(recoverySentinels, get())) {
		if (isNewlineChar(get()))
			delete lexNewline();
		else
			advance();
	}

	throw LexerPanicException();
}

Token* Lexer::lexSingleLineComment() {
	while (!isNewlineChar(get())) advance();
	return lexNewline();
}

Token* Lexer::lexMultiLineComment() {
	auto sourceMeta = getSourceMeta();
	sourceMeta.col--;  // Subtract the column by 1 because we want the column to
					   // start at the '/', not the '*'
	advance();	// The initial '/' has already been read, but we still need to
				// read the initial '*' that proceeds it
	int c;
	while ((c = get()) != EOF) {
		if (isNewlineChar(c)) {
			line++;
			col = 0;
		}
		advance();
		if (c == '*' && get() == '/') {
			advance();
			return nextToken();
		}
	}

	diagnoser.diagnoseMultiLineCommentEnd(sourceMeta);
	panic();
}

Token* Lexer::lexNewline() {
	auto sourceMeta = getSourceMeta();
	StringBuffer sb;
	auto c = get();
	if (c == '\r') sb << (char)advance();
	if (c == '\n') sb << (char)advance();
	col = 1;
	line++;
	String content = sb.str();
	return new Token{TokenType::NL, content, sourceMeta};
}

void Lexer::lexExponent(StringBuffer& sb) {
	sb << (char)advance();	// Skip the [eE]

	if (get() == '+' || get() == '-') sb << (char)advance();

	int c = get();
	if (!isdigit(c)) {
		diagnoser.diagnoseFloatLiteral(getSourceMeta());
		panic();
	}
	sb << (char)advance();

	while (isdigit(get())) sb << (char)advance();
}

Token* Lexer::lexHexLiteral(const SourceMeta& sourceMeta) {
	StringBuffer sb;

	int c = get();
	if (!isxdigit(c)) {
		diagnoser.diagnoseHexLiteral(getSourceMeta());
		panic();
	}
	sb << (char)advance();

	while (isxdigit(get())) sb << (char)advance();

	String content = sb.str();
	return new Token{TokenType::HEX_LITERAL, content, sourceMeta};
}

Token* Lexer::lexOctalLiteral(const SourceMeta& sourceMeta) {
	StringBuffer sb;

	int c = get();
	if (!isOctalDigit(c)) {
		diagnoser.diagnoseOctalLiteral(getSourceMeta());
		panic();
	}
	sb << (char)advance();

	while (isOctalDigit(get())) sb << (char)advance();

	String content = sb.str();
	return new Token{TokenType::OCTAL_LITERAL, content, sourceMeta};
}

Token* Lexer::lexBinaryLiteral(const SourceMeta& sourceMeta) {
	StringBuffer sb;

	int c = get();
	if (!isBinaryDigit(c)) {
		diagnoser.diagnoseBinaryLiteral(getSourceMeta());
		panic();
	}
	sb << (char)advance();

	while (isBinaryDigit(get())) sb << (char)advance();

	String content = sb.str();
	return new Token{TokenType::BINARY_LITERAL, content, sourceMeta};
}

Token* Lexer::lexNumber() {
	auto sourceMeta = getSourceMeta();
	auto initial = advance();

	auto next = get();
	if (initial == '0' && (next == 'x' || next == 'X')) {
		advance();
		return lexHexLiteral(sourceMeta);
	}
	if (initial == '0' && (next == 'o' || next == 'O')) {
		advance();
		return lexOctalLiteral(sourceMeta);
	}
	if (initial == '0' && (next == 'b' || next == 'B')) {
		advance();
		return lexBinaryLiteral(sourceMeta);
	}

	StringBuffer sb;
	sb << (char)initial;
	if (initial == '.') {
		sb.str("0.");
		/*
		We don't need to check for isdigit(next) here because we will only reach
		this condition inside of lexSymbol when it finds a sequence of a dot
		proceeded by a digit
		*/
		while (isdigit(get())) sb << (char)advance();

		if (get() == 'e' || get() == 'E') lexExponent(sb);

		String content = sb.str();
		return new Token{TokenType::FLOAT_LITERAL, content, sourceMeta};
	}

	while (isdigit(get())) sb << (char)advance();

	if (get() == 'e' || get() == 'E') {
		lexExponent(sb);
		String content = sb.str();
		return new Token{TokenType::FLOAT_LITERAL, content, sourceMeta};
	}

	if (get() == '.') {
		sb << (char)advance();

		if (get() == '.') {
			// If the sequence is a number followed by two dots, the two dots
			// need to be tokenized as a single symbol
			retract('.');
			String content = sb.str();
			return new Token{TokenType::INTEGER_LITERAL,
							 content.substr(0, content.length() - 1),
							 sourceMeta};
		}

		while (isdigit(get())) sb << (char)advance();

		if (get() == 'e' || get() == 'E') lexExponent(sb);

		String content = sb.str();

		if (content[content.length() - 1] == '.') content.append("0");

		return new Token{TokenType::FLOAT_LITERAL, content, sourceMeta};
	}

	String content = sb.str();
	return new Token{TokenType::INTEGER_LITERAL, content, sourceMeta};
}

Token* Lexer::lexSymbol() {
	auto initial = get();

	auto sourceMeta = getSourceMeta();
	StringBuffer sb;
	sb << (char)advance();

	auto next = get();

	if (initial == '/' && next == '/') return lexSingleLineComment();
	if (initial == '/' && next == '*') return lexMultiLineComment();
	if (initial == '.' && isdigit(next)) {
		retract(initial);
		return lexNumber();
	}

	while (isSymbolPart(get())) {
		sb << (char)advance();
	}

	String content = sb.str();
	TokenType type = TokenType::EOF_TOKEN;
	int originalLength = content.length();
	while (true) {
		type = getSymbolType(content);
		if (type != TokenType::EOF_TOKEN) break;
		if (content.length() == 1) break;
		char c = content[content.length() - 1];
		content.erase(content.length() - 1);
		retract(c);
	}
	if (type == TokenType::EOF_TOKEN) {
		diagnoser.diagnose(ec::INVALID_LEXICAL_SYMBOL, sourceMeta,
						   originalLength);
		panic();
	}

	return new Token{type, content, sourceMeta};
}

Token* Lexer::lexIdentifier() {
	auto sourceMeta = getSourceMeta();

	StringBuffer sb;
	sb << (char)advance();
	while (isIdentifierPart(get())) sb << (char)advance();

	String content = sb.str();
	TokenType type = getIdentifierType(content);
	if (type == TokenType::TRY && get() == '?') {
		type = TokenType::TRY_OPTIONAL;
		content.append(1, (char)advance());
	} else if (type == TokenType::TRY && get() == '!') {
		type = TokenType::TRY_UNWRAPPED;
		content.append(1, (char)advance());
	} else if (type == TokenType::AS && get() == '?') {
		type = TokenType::AS_OPTIONAL;
		content.append(1, (char)advance());
	} else if (type == TokenType::AS && get() == '!') {
		type = TokenType::AS_UNWRAPPED;
		content.append(1, (char)advance());
	}

	return new Token{type, content, sourceMeta};
}

Token* Lexer::lexMeta() {
	auto sourceMeta = getSourceMeta();

	StringBuffer sb;
	sb << (char)advance();
	auto t = lexIdentifier();

	sb << t->data;

	delete t;

	String content = sb.str();

	auto type = getMetaType(content);

	if (type == TokenType::EOF_TOKEN) {
		diagnoser.diagnose(ec::INVALID_TAG, sourceMeta,
						   content.length() + 1);  // +1 for the initial '@'
		panic();
	}

	return new Token{type, content, sourceMeta};
}

void Lexer::lexUnicodeEscapeSequence(StringBuffer& sb, int n) {
	for (int i = 0; i < n; i++) {
		int c = get();
		if (!isxdigit(c)) {
			diagnoser.diagnose(ec::INVALID_UNICODE_ESCAPE_SEQUENCE,
							   getSourceMeta(), 1);
			panic();
		}
		sb << (char)advance();
	}
}

void Lexer::lexOctalEscapeSequence(StringBuffer& sb) {
	for (int i = 0; i < 3; i++) {
		int c = get();
		if (!isOctalDigit(c)) return;
		sb << (char)advance();
	}
}

// TODO: Fix the interpolation to allow for nested strings
void Lexer::lexInterpolationEscapeSequence(int pos,
										   Map<int, String>& interpolations) {
	StringBuffer sb;
	advance();
	int lbraceCount = 1;
	auto c = get();
	bool prevWasCR = false;
	while ((c = get()) != EOF) {
		if (c == '{')
			lbraceCount++;
		else if (c == '}')
			lbraceCount--;

		if (lbraceCount == 0) {
			interpolations[pos] = sb.str();
			return;
		}

		if (c == '\r' || (c == '\n' && !prevWasCR)) {
			line++;
			prevWasCR = c == '\r';
		} else
			prevWasCR = false;

		sb << (char)advance();
	}

	diagnoser.diagnose(ec::INVALID_INTERPOLATION, getSourceMeta(), 1);
	panic();
}

void Lexer::lexEscapeSequence(StringBuffer& sb,
							  Map<int, String>& interpolations) {
	sb << (char)advance();
	int c = get();
	if (isSimpleEscapeCharacter(c)) {
		sb << (char)advance();
	} else if (c == 'u') {
		sb << (char)advance();
		lexUnicodeEscapeSequence(sb, 4);
	} else if (c == 'U') {
		sb << (char)advance();
		lexUnicodeEscapeSequence(sb, 8);
	} else if (isOctalDigit(c)) {
		lexOctalEscapeSequence(sb);
	} else if (c == '{') {
		sb.get();  // Remove the backslash character
		auto pos =
			getStringBufferLength(sb);	// Position of interpolation insertion
		lexInterpolationEscapeSequence(pos, interpolations);
	} else {
		diagnoser.diagnose(ec::INVALID_ESCAPE_SEQUENCE, getSourceMeta(), 1);
		panic();
	}
}

Token* Lexer::lexString(int delimiter) {
	auto sourceMeta = getSourceMeta();
	advance();
	StringBuffer sb;
	int c;
	Map<int, String> interpolations;
	while ((c = get()) != delimiter && c != EOF) {
		if (c == '\\') {
			lexEscapeSequence(sb, interpolations);
		} else {
			sb << (char)c;
			advance();
		}
	}
	if (c == EOF) {
		diagnoser.diagnose(ec::INVALID_STRING_LITERAL_END, sourceMeta, 1);
		panic();
	}
	advance();
	String content = sb.str();
	return new StringToken{TokenType::STRING_LITERAL, content, sourceMeta,
						   interpolations};
}

Token* Lexer::nextToken() {
	while (isblank(get())) advance();

	if (!hasNext())
		return new Token{TokenType::EOF_TOKEN, "(EOF)", getSourceMeta()};

	auto c = get();
	if (isSymbolStart(c)) return lexSymbol();
	if (isdigit(c)) return lexNumber();
	if (isIdentifierStart(c)) return lexIdentifier();
	if (c == '\'' || c == '"') return lexString(c);
	if (c == '@') return lexMeta();
	if (isNewlineChar(c)) return lexNewline();

	diagnoser.diagnose(ec::INVALID_INPUT, getSourceMeta(), 1);
	panic();
}

bool Lexer::hasNext() const { return buf.rdbuf()->in_avail() > 0; }

const ModuleInfo& Lexer::getModuleInfo() const { return moduleInfo; }

void Lexer::setRecoverySentinels(const List<int>& sentinels) {
	recoverySentinels.clear();
	recoverySentinels.insert(recoverySentinels.end(), sentinels.begin(),
							 sentinels.end());
}
}  // namespace acl

namespace acl {
bool isSymbolStart(int c) {
	String chars = "~!%^&*()-+=[]{}|:<>,./?;";
	return chars.find(c) != String::npos;
}

bool isSymbolPart(int c) {
	String chars = "&*-+=|<>?.";
	return chars.find(c) != String::npos;
}

bool isIdentifierStart(int c) { return isalpha(c) || c == '_' || c == '$'; }

bool isIdentifierPart(int c) { return isIdentifierStart(c) || isdigit(c); }

bool isSimpleEscapeCharacter(int c) {
	String str = "abfnrtv'\"\\";
	return str.find(c) != String::npos;
}

bool isOctalDigit(int c) { return c >= '0' && c <= '7'; }

bool isBinaryDigit(int c) { return c == '0' || c == '1'; }

bool isNewlineChar(int c) { return c == '\r' || c == '\n'; }

TokenType getIdentifierType(const String& str) {
	if (str == "public") return TokenType::PUBLIC;
	if (str == "private") return TokenType::PRIVATE;
	if (str == "protected") return TokenType::PROTECTED;
	if (str == "internal") return TokenType::INTERNAL;
	if (str == "static") return TokenType::STATIC;
	if (str == "unsafe") return TokenType::UNSAFE;
	if (str == "atomic") return TokenType::ATOMIC;
	if (str == "ref") return TokenType::REF;
	if (str == "strong") return TokenType::STRONG;
	if (str == "weak") return TokenType::WEAK;
	if (str == "greedy") return TokenType::GREEDY;
	if (str == "final") return TokenType::FINAL;
	if (str == "override") return TokenType::OVERRIDE;
	if (str == "infix") return TokenType::INFIX;
	if (str == "prefix") return TokenType::PREFIX;
	if (str == "postfix") return TokenType::POSTFIX;
	if (str == "class") return TokenType::CLASS;
	if (str == "struct") return TokenType::STRUCT;
	if (str == "template") return TokenType::TEMPLATE;
	if (str == "enum") return TokenType::ENUM;
	if (str == "namespace") return TokenType::NAMESPACE;
	if (str == "var") return TokenType::VAR;
	if (str == "const") return TokenType::CONST;
	if (str == "fun") return TokenType::FUN;
	if (str == "self") return TokenType::SELF;
	if (str == "super") return TokenType::SUPER;
	if (str == "if") return TokenType::IF;
	if (str == "elif") return TokenType::ELIF;
	if (str == "else") return TokenType::ELSE;
	if (str == "for") return TokenType::FOR;
	if (str == "in") return TokenType::IN;
	if (str == "while") return TokenType::WHILE;
	if (str == "repeat") return TokenType::REPEAT;
	if (str == "switch") return TokenType::SWITCH;
	if (str == "case") return TokenType::CASE;
	if (str == "default") return TokenType::DEFAULT;
	if (str == "break") return TokenType::BREAK;
	if (str == "continue") return TokenType::CONTINUE;
	if (str == "return") return TokenType::RETURN;
	if (str == "throw") return TokenType::THROW;
	if (str == "throwing") return TokenType::THROWING;
	if (str == "noexcept") return TokenType::NOEXCEPT;
	if (str == "import") return TokenType::IMPORT;
	if (str == "from") return TokenType::FROM;
	if (str == "as") return TokenType::AS;
	if (str == "async") return TokenType::ASYNC;
	if (str == "await") return TokenType::AWAIT;
	if (str == "release") return TokenType::RELEASE;
	if (str == "get") return TokenType::GET;
	if (str == "set") return TokenType::SET;
	if (str == "init") return TokenType::INIT;
	if (str == "construct") return TokenType::CONSTRUCT;
	if (str == "destruct") return TokenType::DESTRUCT;
	if (str == "try") return TokenType::TRY;
	if (str == "catch") return TokenType::CATCH;
	if (str == "and") return TokenType::AND;
	if (str == "or") return TokenType::OR;
	if (str == "not") return TokenType::NOT;
	if (str == "alias") return TokenType::ALIAS;
	if (str == "extern") return TokenType::EXTERN;
	if (str == "fall") return TokenType::FALL;
	if (str == "global") return TokenType::GLOBAL;
	if (str == "is") return TokenType::IS;
	if (str == "uses") return TokenType::USES;
	if (str == "true" || str == "false") return TokenType::BOOLEAN_LITERAL;
	if (str == "nil") return TokenType::NIL_LITERAL;

	return TokenType::ID;
}

TokenType getSymbolType(const String& str) {
	if (str == "~") return TokenType::TILDE;
	if (str == "!") return TokenType::EXCLAMATION_POINT;
	if (str == "%") return TokenType::PERCENT;
	if (str == "^") return TokenType::CARET;
	if (str == "&") return TokenType::AMPERSAND;
	if (str == "*") return TokenType::ASTERISK;
	if (str == "(") return TokenType::LPAREN;
	if (str == ")") return TokenType::RPAREN;
	if (str == "-") return TokenType::MINUS;
	if (str == "=") return TokenType::EQUALS;
	if (str == "+") return TokenType::PLUS;
	if (str == "[") return TokenType::LBRACKET;
	if (str == "]") return TokenType::RBRACKET;
	if (str == "{") return TokenType::LBRACE;
	if (str == "}") return TokenType::RBRACE;
	if (str == "|") return TokenType::PIPE;
	if (str == ":") return TokenType::COLON;
	if (str == "<") return TokenType::LT;
	if (str == ">") return TokenType::GT;
	if (str == ",") return TokenType::COMMA;
	if (str == ".") return TokenType::DOT;
	if (str == "/") return TokenType::SLASH;
	if (str == "?") return TokenType::QUESTION_MARK;
	if (str == "==") return TokenType::DOUBLE_EQUALS;
	if (str == "===") return TokenType::TRIPLE_EQUALS;
	if (str == "~=") return TokenType::TILDE_EQUALS;
	if (str == "!=") return TokenType::EXCLAMATION_POINT_EQUALS;
	if (str == "!==") return TokenType::EXCLAMATION_POINT_DOUBLE_EQUALS;
	if (str == "%=") return TokenType::PERCENT_EQUALS;
	if (str == "^=") return TokenType::CARET_EQUALS;
	if (str == "&=") return TokenType::AMPERSAND_EQUALS;
	if (str == "*=") return TokenType::ASTERISK_EQUALS;
	if (str == "-=") return TokenType::MINUS_EQUALS;
	if (str == "+=") return TokenType::PLUS_EQUALS;
	if (str == "|=") return TokenType::PIPE_EQUALS;
	if (str == "<=") return TokenType::LT_EQUALS;
	if (str == ">=") return TokenType::GT_EQUALS;
	if (str == "/=") return TokenType::SLASH_EQUALS;
	if (str == "&&") return TokenType::DOUBLE_AMPERSAND;
	if (str == "||") return TokenType::DOUBLE_PIPE;
	if (str == "**") return TokenType::DOUBLE_ASTERISK;
	if (str == "--") return TokenType::DOUBLE_MINUS;
	if (str == "++") return TokenType::DOUBLE_PLUS;
	if (str == "<<") return TokenType::DOUBLE_LT;
	if (str == ">>") return TokenType::DOUBLE_GT;
	if (str == "..") return TokenType::DOUBLE_DOT;
	if (str == "...") return TokenType::TRIPLE_DOT;
	if (str == "??") return TokenType::DOUBLE_QUESTION_MARK;
	if (str == "?.") return TokenType::QUESTION_MARK_DOT;
	if (str == "->") return TokenType::MINUS_ARROW;
	if (str == "=>") return TokenType::EQUALS_ARROW;
	if (str == ";") return TokenType::SEMICOLON;
	if (str == "<=>") return TokenType::COMPARE;
	if (str == "**=") return TokenType::DOUBLE_ASTERISK_EQUALS;
	if (str == "<<=") return TokenType::DOUBLE_LT_EQUALS;
	if (str == ">>=") return TokenType::DOUBLE_GT_EQUALS;

	return TokenType::EOF_TOKEN;
}

TokenType getMetaType(const String& str) {
	if (str == "@noreturn") return TokenType::META_NORETURN;
	if (str == "@stackalloc") return TokenType::META_STACKALLOC;
	if (str == "@srclock") return TokenType::META_SRCLOCK;
	if (str == "@laxthrow") return TokenType::META_LAXTHROW;
	if (str == "@externalinit") return TokenType::META_EXTERNALINIT;
	if (str == "@deprecated") return TokenType::META_DEPRECATED;
	if (str == "@enablewarning") return TokenType::META_ENABLEWARNING;
	if (str == "@disablewarning") return TokenType::META_DISABLEWARNING;
	if (str == "@nobuiltins") return TokenType::META_NOBUILTINS;

	return TokenType::EOF_TOKEN;
}

int getStringBufferLength(StringBuffer& buf) {
	buf.seekg(0, std::ios::end);
	int len = buf.tellg();
	buf.seekg(0, std::ios::beg);
	return len;
}

Relexer::Relexer(const CompilerContext& ctx, Token* originalToken)
	: ctx(ctx), originalToken(originalToken) {}

void Relexer::tryLex(const String& str, List<Token*>& dest) {
	StringBuffer buf;
	buf << str;
	Lexer lexer = Lexer(ctx, *originalToken->meta.moduleInfo, buf);
	while (lexer.hasNext()) {
		try {
			dest.push_back(lexer.nextToken());
		} catch (AclException& e) {
			dest.clear();
			return;
		}
	}
}

Token* Relexer::formatToken(Token* t, int start) {
	t->meta.moduleInfo = originalToken->meta.moduleInfo;
	t->meta.line = originalToken->meta.line;
	t->meta.col = originalToken->meta.col + start;
	return t;
}

void Relexer::relexHelper(int current, List<Token*>& dest) {
	int start = current;
	StringBuffer sb;
	sb << originalToken->data[current++];
	List<Token*> tmp;
	tryLex(sb.str(), tmp);
	while (tmp.empty() && (std::size_t)current < originalToken->data.length()) {
		sb << originalToken->data[current++];
		tryLex(sb.str(), tmp);
	}
	if (tmp.empty()) {
		dest.clear();
		return;
	}
	for (auto& t : tmp) dest.push_back(formatToken(t, start));
	if ((std::size_t)current < originalToken->data.length())
		relexHelper(current, dest);
}

void Relexer::relex(List<Token*>& dest) { relexHelper(0, dest); }

String getStringForTokenType(TokenType type) {
	return TOKEN_TYPE_STRINGS.at(type);
}
}  // namespace acl