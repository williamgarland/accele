#include "lexer.hpp"
#include "exceptions.hpp"

namespace acl {
Token::Token(TokenType type, const String& data, const SourceMeta& meta):
    type(type), data(data), meta(meta) {}
Token::~Token() {}

StringToken::StringToken(TokenType type, const String& data, const SourceMeta& meta, const Map<int, String>& interpolations):
    Token(type, data, meta), interpolations(interpolations) {}
StringToken::~StringToken() {}

SourceMeta Lexer::getSourceMeta() {
    return { file, line, col };
}

int Lexer::get() {
    return buf.peek();
}

int Lexer::advance() {
    col++;
    return buf.get();
}

void Lexer::retract(char c) {
    col--;
    buf.putback(c);
}

Token* Lexer::lexComment() {
    return nullptr;
}

Token* Lexer::lexNewline() {
    return nullptr;
}

Token* Lexer::lexNumber() {

}

Token* Lexer::lexSymbol() {
    auto sourceMeta = getSourceMeta();
    StringBuffer sb;
    sb << (char) advance();
    while (isSymbolPart(get()))
        sb << (char) advance();
    
    String content = sb.str();
    TokenType type = TokenType::EOF_TOKEN;
    InvalidInputException err;
    while (true) {
        try {
            type = getSymbolType(sourceMeta, content);
            break;
        } catch (InvalidInputException& e) {
            err = e;
            if (content.length() == 1)
                break;
            char c = content[content.length() - 1];
            content.erase(content.length() - 1);
            retract(c);
        }
    }
    if (type == TokenType::EOF_TOKEN)
        throw err;
    
    return new Token{ type, content, sourceMeta };
}

Token* Lexer::lexIdentifier() {
    auto sourceMeta = getSourceMeta();
            
    StringBuffer sb;
    sb << (char) advance();
    while (isIdentifierPart(get()))
        sb << (char) advance();
    
    String content = sb.str();
    TokenType type = getIdentifierType(content);
    if (type == TokenType::TRY && get() == '?') {
        type = TokenType::TRY_OPTIONAL;
        content.append(1, (char) advance());
    } else if (type == TokenType::TRY && get() == '!') {
        type = TokenType::TRY_UNWRAPPED;
        content.append(1, (char) advance());
    } else if (type == TokenType::AS && get() == '?') {
        type = TokenType::AS_OPTIONAL;
        content.append(1, (char) advance());
    } else if (type == TokenType::AS && get() == '!') {
        type = TokenType::AS_UNWRAPPED;
        content.append(1, (char) advance());
    }
    
    return new Token{ type, content, sourceMeta };
}

Token* Lexer::lexMeta() {
    return nullptr;
}

void Lexer::lexUnicodeEscapeSequence(StringBuffer& sb, int n) {
    for (int i = 0; i < n; i++) {
        int c = get();
        if (!isxdigit(c))
            throw LexerException(getSourceMeta(), "Invalid Unicode escape sequence");
        sb << (char) advance();
    }
}

void Lexer::lexOctalEscapeSequence(StringBuffer& sb) {
    for (int i = 0; i < 3; i++) {
        int c = get();
        if (!isOctalDigit(c))
            return;
        sb << (char) advance();
    }
}

void Lexer::lexInterpolationEscapeSequence(int pos, Map<int, String>& interpolations) {
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
        
        sb << (char) advance();
    }

    throw LexerException(getSourceMeta(), "Invalid interpolation escape sequence");
}

void Lexer::lexEscapeSequence(StringBuffer& sb, Map<int, String>& interpolations) {
    sb << (char) advance();
    int c = get();
    if (isSimpleEscapeCharacter(c)) {
        sb << (char) advance();
    } else if (c == 'u') {
        sb << (char) advance();
        lexUnicodeEscapeSequence(sb, 4);
    } else if (c == 'U') {
        sb << (char) advance();
        lexUnicodeEscapeSequence(sb, 8);
    } else if (isOctalDigit(c)) {
        lexOctalEscapeSequence(sb);
    } else if (c == '{') {
        sb.get(); // Remove the backslash character
        auto pos = getStringBufferLength(sb); // Position of interpolation insertion
        lexInterpolationEscapeSequence(pos, interpolations);
    } else {
        throw InvalidInputException(getSourceMeta(), c);
    }
}

Token* Lexer::lexString(int delimiter) {
    auto sourceMeta = getSourceMeta();
    StringBuffer sb;
    int c;
    Map<int, String> interpolations;
    while ((c = get()) != delimiter) {
        if (c == '\\') {
            lexEscapeSequence(sb, interpolations);
        } else
            sb << (char) c;
    }
    advance();
    String content = sb.str();
    return new StringToken{ TokenType::STRING_LITERAL, content, sourceMeta, interpolations };
}

Token* Lexer::nextToken() {
    auto c = get();
    if (isSymbolStart(c))
        return lexSymbol();
    if (isdigit(c))
        return lexNumber();
    if (isIdentifierStart(c))
        return lexIdentifier();
    if (c == '\'' || c == '"')
        return lexString(c);
    if (c == '@')
        return lexMeta();
    throw InvalidInputException(getSourceMeta(), c);
}

bool Lexer::hasNext() const {
    return buf.rdbuf()->in_avail() > 0;
}
}

namespace acl {
bool isSymbolStart(int c) {
    String chars = "~!%^&*()-+=[]{}|:<>,./?;";
    return chars.find(c) != String::npos;
}

bool isSymbolPart(int c) {
    String chars = "&*-+=|<>?";
    return chars.find(c) != String::npos;
}

bool isIdentifierStart(int c) {
    return isalpha(c) || c == '_' || c == '$';
}

bool isIdentifierPart(int c) {
    return isIdentifierStart(c) || isdigit(c);
}

bool isSimpleEscapeCharacter(int c) {
    String str = "abfnrtv'\"\\";
    return str.find(c) != String::npos;
}

bool isOctalDigit(int c) {
    return c >= '0' && c <= '7';
}

TokenType getIdentifierType(const String& str) {
    if (str == "public")
        return TokenType::PUBLIC;
    if (str == "private")
        return TokenType::PRIVATE;
    if (str == "protected")
        return TokenType::PROTECTED;
    if (str == "internal")
        return TokenType::INTERNAL;
    if (str == "static")
        return TokenType::STATIC;
    if (str == "unsafe")
        return TokenType::UNSAFE;
    if (str == "atomic")
        return TokenType::ATOMIC;
    if (str == "ref")
        return TokenType::REF;
    if (str == "strong")
        return TokenType::STRONG;
    if (str == "weak")
        return TokenType::WEAK;
    if (str == "greedy")
        return TokenType::GREEDY;
    if (str == "final")
        return TokenType::FINAL;
    if (str == "override")
        return TokenType::OVERRIDE;
    if (str == "infix")
        return TokenType::INFIX;
    if (str == "prefix")
        return TokenType::PREFIX;
    if (str == "postfix")
        return TokenType::POSTFIX;
    if (str == "class")
        return TokenType::CLASS;
    if (str == "struct")
        return TokenType::STRUCT;
    if (str == "template")
        return TokenType::TEMPLATE;
    if (str == "enum")
        return TokenType::ENUM;
    if (str == "namespace")
        return TokenType::NAMESPACE;
    if (str == "var")
        return TokenType::VAR;
    if (str == "const")
        return TokenType::CONST;
    if (str == "fun")
        return TokenType::FUN;
    if (str == "self")
        return TokenType::SELF;
    if (str == "super")
        return TokenType::SUPER;
    if (str == "if")
        return TokenType::IF;
    if (str == "elif")
        return TokenType::ELIF;
    if (str == "else")
        return TokenType::ELSE;
    if (str == "for")
        return TokenType::FOR;
    if (str == "in")
        return TokenType::IN;
    if (str == "while")
        return TokenType::WHILE;
    if (str == "repeat")
        return TokenType::REPEAT;
    if (str == "switch")
        return TokenType::SWITCH;
    if (str == "case")
        return TokenType::CASE;
    if (str == "default")
        return TokenType::DEFAULT;
    if (str == "break")
        return TokenType::BREAK;
    if (str == "continue")
        return TokenType::CONTINUE;
    if (str == "return")
        return TokenType::RETURN;
    if (str == "throw")
        return TokenType::THROW;
    if (str == "throwing")
        return TokenType::THROWING;
    if (str == "noexcept")
        return TokenType::NOEXCEPT;
    if (str == "import")
        return TokenType::IMPORT;
    if (str == "from")
        return TokenType::FROM;
    if (str == "as")
        return TokenType::AS;
    if (str == "async")
        return TokenType::ASYNC;
    if (str == "await")
        return TokenType::AWAIT;
    if (str == "release")
        return TokenType::RELEASE;
    if (str == "get")
        return TokenType::GET;
    if (str == "set")
        return TokenType::SET;
    if (str == "init")
        return TokenType::INIT;
    if (str == "construct")
        return TokenType::CONSTRUCT;
    if (str == "destruct")
        return TokenType::DESTRUCT;
    if (str == "try")
        return TokenType::TRY;
    if (str == "catch")
        return TokenType::CATCH;
    if (str == "and")
        return TokenType::AND;
    if (str == "or")
        return TokenType::OR;
    if (str == "not")
        return TokenType::NOT;
    if (str == "alias")
        return TokenType::ALIAS;
    if (str == "extern")
        return TokenType::EXTERN;
    if (str == "fall")
        return TokenType::FALL;
    if (str == "global")
        return TokenType::GLOBAL;
        
    return TokenType::ID;
}

TokenType getSymbolType(const SourceMeta& sourceMeta, const String& str) {
    if (str == "~")
        return TokenType::TILDE;
    if (str == "!")
        return TokenType::EXCLAMATION_POINT;
    if (str == "%")
        return TokenType::PERCENT;
    if (str == "^")
        return TokenType::CARET;
    if (str == "&")
        return TokenType::AMPERSAND;
    if (str == "*")
        return TokenType::ASTERISK;
    if (str == "(")
        return TokenType::LPAREN;
    if (str == ")")
        return TokenType::RPAREN;
    if (str == "-")
        return TokenType::MINUS;
    if (str == "=")
        return TokenType::EQUALS;
    if (str == "+")
        return TokenType::PLUS;
    if (str == "[")
        return TokenType::LBRACKET;
    if (str == "]")
        return TokenType::RBRACKET;
    if (str == "{")
        return TokenType::LBRACE;
    if (str == "}")
        return TokenType::RBRACE;
    if (str == "|")
        return TokenType::PIPE;
    if (str == ":")
        return TokenType::COLON;
    if (str == "<")
        return TokenType::LT;
    if (str == ">")
        return TokenType::GT;
    if (str == ",")
        return TokenType::COMMA;
    if (str == ".")
        return TokenType::DOT;
    if (str == "/")
        return TokenType::SLASH;
    if (str == "?")
        return TokenType::QUESTION_MARK;
    if (str == "==")
        return TokenType::DOUBLE_EQUALS;
    if (str == "===")
        return TokenType::TRIPLE_EQUALS;
    if (str == "~=")
        return TokenType::TILDE_EQUALS;
    if (str == "!=")
        return TokenType::EXCLAMATION_POINT_EQUALS;
    if (str == "!==")
        return TokenType::EXCLAMATION_POINT_DOUBLE_EQUALS;
    if (str == "%=")
        return TokenType::PERCENT_EQUALS;
    if (str == "^=")
        return TokenType::CARET_EQUALS;
    if (str == "&=")
        return TokenType::AMPERSAND_EQUALS;
    if (str == "*=")
        return TokenType::ASTERISK_EQUALS;
    if (str == "-=")
        return TokenType::MINUS_EQUALS;
    if (str == "+=")
        return TokenType::PLUS_EQUALS;
    if (str == "|=")
        return TokenType::PIPE_EQUALS;
    if (str == "<=")
        return TokenType::LT_EQUALS;
    if (str == ">=")
        return TokenType::GT_EQUALS;
    if (str == "/=")
        return TokenType::SLASH_EQUALS;
    if (str == "&&")
        return TokenType::DOUBLE_AMPERSAND;
    if (str == "||")
        return TokenType::DOUBLE_PIPE;
    if (str == "**")
        return TokenType::DOUBLE_ASTERISK;
    if (str == "--")
        return TokenType::DOUBLE_MINUS;
    if (str == "++")
        return TokenType::DOUBLE_PLUS;
    if (str == "<<")
        return TokenType::DOUBLE_LT;
    if (str == ">>")
        return TokenType::DOUBLE_GT;
    if (str == "..")
        return TokenType::DOUBLE_DOT;
    if (str == "...")
        return TokenType::TRIPLE_DOT;
    if (str == "??")
        return TokenType::DOUBLE_QUESTION_MARK;
    if (str == "?.")
        return TokenType::QUESTION_MARK_DOT;
    if (str == "->")
        return TokenType::MINUS_ARROW;
    if (str == "=>")
        return TokenType::EQUALS_ARROW;
    if (str == ";")
        return TokenType::SEMICOLON;
    if (str == "<=>")
        return TokenType::COMPARE;
    if (str == "**=")
        return TokenType::DOUBLE_ASTERISK_EQUALS;
    if (str == "<<=")
        return TokenType::DOUBLE_LT_EQUALS;
    if (str == ">>=")
        return TokenType::DOUBLE_GT_EQUALS;
        
    throw InvalidInputException(sourceMeta, str[0]);
}

TokenType getMetaType(const SourceMeta& sourceMeta, const String& str) {
    if (str == "@noreturn")
        return TokenType::META_NORETURN;
    if (str == "@stackalloc")
        return TokenType::META_STACKALLOC;
    if (str == "@srclock")
        return TokenType::META_SRCLOCK;
    throw InvalidInputException(sourceMeta, str[0]);
}

int getStringBufferLength(StringBuffer& buf) {
    buf.seekg(0, std::ios::end);
    int len = buf.tellg();
    buf.seekg(0, std::ios::beg);
    return len;
}
}