#pragma once

#include "common.hpp"

namespace acl {
enum class TokenType {
	// ----- Keywords ----- //
	PUBLIC,
	PRIVATE,
	PROTECTED,
	INTERNAL,
	STATIC,
	UNSAFE,
	ATOMIC,
	REF,
	STRONG,
	WEAK,
	GREEDY,
	FINAL,
	OVERRIDE,
	INFIX,
	PREFIX,
	POSTFIX,
	CLASS,
	STRUCT,
	TEMPLATE,
	ENUM,
	NAMESPACE,
	VAR,
	CONST,
	FUN,
	SELF,
	SUPER,
	IF,
	ELIF,
	ELSE,
	FOR,
	IN,
	WHILE,
	REPEAT,
	SWITCH,
	CASE,
	DEFAULT,
	BREAK,
	CONTINUE,
	RETURN,
	THROW,
	THROWING,
	NOEXCEPT,
	IMPORT,
	FROM,
	AS,
	ASYNC,
	AWAIT,
	RELEASE,
	GET,
	SET,
	INIT,
	CONSTRUCT,
	DESTRUCT,
	TRY,
	CATCH,
	AND,
	OR,
	NOT,
	AS_OPTIONAL,
	AS_UNWRAPPED,
	TRY_OPTIONAL,
	TRY_UNWRAPPED,
	ALIAS,
	EXTERN,
	FALL,
	GLOBAL,
	IS,
	USES,

	// ----- Symbols ----- //
	TILDE,
	EXCLAMATION_POINT,
	PERCENT,
	CARET,
	AMPERSAND,
	ASTERISK,
	LPAREN,
	RPAREN,
	MINUS,
	EQUALS,
	PLUS,
	LBRACKET,
	RBRACKET,
	LBRACE,
	RBRACE,
	PIPE,
	COLON,
	LT,
	GT,
	COMMA,
	DOT,
	SLASH,
	QUESTION_MARK,
	DOUBLE_EQUALS,
	TRIPLE_EQUALS,
	TILDE_EQUALS,
	EXCLAMATION_POINT_EQUALS,
	EXCLAMATION_POINT_DOUBLE_EQUALS,
	PERCENT_EQUALS,
	CARET_EQUALS,
	AMPERSAND_EQUALS,
	ASTERISK_EQUALS,
	MINUS_EQUALS,
	PLUS_EQUALS,
	PIPE_EQUALS,
	LT_EQUALS,
	GT_EQUALS,
	SLASH_EQUALS,
	DOUBLE_AMPERSAND,
	DOUBLE_PIPE,
	DOUBLE_ASTERISK,
	DOUBLE_MINUS,
	DOUBLE_PLUS,
	DOUBLE_LT,
	DOUBLE_GT,
	DOUBLE_DOT,
	TRIPLE_DOT,
	DOUBLE_QUESTION_MARK,
	QUESTION_MARK_DOT,
	MINUS_ARROW,
	EQUALS_ARROW,
	SEMICOLON,
	COMPARE,
	DOUBLE_ASTERISK_EQUALS,
	DOUBLE_LT_EQUALS,
	DOUBLE_GT_EQUALS,

	// ----- Miscellaneous ----- //
	ID,
	BOOLEAN_LITERAL,
	INTEGER_LITERAL,
	HEX_LITERAL,
	OCTAL_LITERAL,
	BINARY_LITERAL,
	FLOAT_LITERAL,
	STRING_LITERAL,
	NIL_LITERAL,
	EOF_TOKEN,
	NL,

	// ----- Meta ----- //
	META_NORETURN,
	META_STACKALLOC,
	META_SRCLOCK,
	META_LAXTHROW,
	META_EXTERNALINIT,
	META_DEPRECATED,
	META_ENABLEWARNING,
	META_DISABLEWARNING,
	META_NOBUILTINS
};

struct Token {
	TokenType type;
	String data;
	SourceMeta meta;

	Token(TokenType type, const String& data, const SourceMeta& meta);
	virtual ~Token();
};

struct StringToken : public Token {
	Map<int, String> interpolations;

	StringToken(TokenType type, const String& data, const SourceMeta& meta,
				const Map<int, String>& interpolations);
	virtual ~StringToken();
};

class Lexer {
	String file;
	StringBuffer& buf;
	int line;
	int col;

   private:
	SourceMeta getSourceMeta();
	int get();
	int advance();
	void retract(char c);

   private:
	Token* lexSingleLineComment();
	Token* lexMultiLineComment();
	Token* lexNewline();
	void lexExponent(StringBuffer& sb);
	Token* lexHexLiteral(const SourceMeta& sourceMeta);
	Token* lexOctalLiteral(const SourceMeta& sourceMeta);
	Token* lexBinaryLiteral(const SourceMeta& sourceMeta);
	Token* lexNumber();
	Token* lexSymbol();
	Token* lexIdentifier();
	Token* lexMeta();
	void lexUnicodeEscapeSequence(StringBuffer& sb, int n);
	void lexOctalEscapeSequence(StringBuffer& sb);
	void lexInterpolationEscapeSequence(int pos,
										Map<int, String>& interpolations);
	void lexEscapeSequence(StringBuffer& sb, Map<int, String>& interpolations);
	Token* lexString(int delimiter);

   public:
	Lexer(const String& file, StringBuffer& buf);
	Token* nextToken();
	bool hasNext() const;
	const String& getModulePath() const;
};

class Relexer {
	Token* originalToken;

	void tryLex(const String& str, List<Token*>& dest);
	Token* formatToken(Token* t, int start);
	void relexHelper(int current, List<Token*>& dest);

   public:
	Relexer(Token* originalToken);
	void relex(List<Token*>& dest);
};

bool isOctalDigit(int c);
bool isBinaryDigit(int c);
bool isSimpleEscapeCharacter(int c);
bool isIdentifierStart(int c);
bool isIdentifierPart(int c);
bool isSymbolStart(int c);
bool isSymbolPart(int c);
bool isNewlineChar(int c);

TokenType getIdentifierType(const String& str);
TokenType getSymbolType(const SourceMeta& sourceMeta, const String& str);
TokenType getMetaType(const SourceMeta& sourceMeta, const String& str);

int getStringBufferLength(StringBuffer& buf);
}  // namespace acl