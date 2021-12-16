#include <string>
#include <sstream>
#include <vector>

namespace acl {
    typedef std::string String;
    typedef std::stringstream StringBuffer;
    
    template <typename T>
    using List = std::vector<T>;
    
    class Parser {

    };

    class Node {
    public:
        virtual ~Node() {}

        virtual void parse(Parser& parser) = 0;
    };

    class Expression: public Node {
    public:
        virtual ~Expression() {}

        virtual void parse(Parser& parser) {

        }
    };

    class BinaryExpression: public Expression {
    protected:
        Expression* left;
        Expression* right;
    public:
        BinaryExpression(Expression* left, Expression* right): left(left), right(right) {}
        virtual ~BinaryExpression() { delete left; delete right; }
    };

    class AccessExpression: public BinaryExpression {
    public:
        AccessExpression(Expression* left, Expression* right): BinaryExpression(left, right) {}
        virtual ~AccessExpression() {}
    };

    class IdentifierExpression: public Expression {

    };

    enum class TokenType {
        // ----- Keywords ----- //
        PUBLIC, PRIVATE, PROTECTED, INTERNAL, STATIC, UNSAFE, ATOMIC, REF, STRONG, WEAK, GREEDY,
        FINAL, OVERRIDE, INFIX, PREFIX, POSTFIX, CLASS, STRUCT, TEMPLATE, ENUM, NAMESPACE, VAR,
        CONST, FUN, SELF, SUPER, IF, ELIF, ELSE, FOR, IN, WHILE, REPEAT, SWITCH, CASE, DEFAULT,
        BREAK, CONTINUE, RETURN, THROW, THROWING, NOEXCEPT, IMPORT, FROM, AS, ASYNC, AWAIT,
        RELEASE, GET, SET, INIT, CONSTRUCT, DESTRUCT, TRY, CATCH, AND, OR, NOT, AS_OPTIONAL,
        AS_UNWRAPPED, TRY_OPTIONAL, TRY_UNWRAPPED, ALIAS, EXTERN, FALL,

        // ----- Symbols ----- //
        TILDE, EXCLAMATION_POINT, PERCENT, CARET, AMPERSAND, ASTERISK, LPAREN, RPAREN, MINUS,
        EQUALS, PLUS, LBRACKET, RBRACKET, LBRACE, RBRACE, PIPE, COLON, LT, GT, COMMA, DOT, SLASH,
        QUESTION_MARK, DOUBLE_EQUALS, TRIPLE_EQUALS, TILDE_EQUALS, EXCLAMATION_POINT_EQUALS,
        EXCLAMATION_POINT_DOUBLE_EQUALS, PERCENT_EQUALS, CARET_EQUALS, AMPERSAND_EQUALS,
        ASTERISK_EQUALS, MINUS_EQUALS, PLUS_EQUALS, PIPE_EQUALS, LT_EQUALS, GT_EQUALS,
        SLASH_EQUALS, DOUBLE_AMPERSAND, DOUBLE_PIPE, DOUBLE_ASTERISK, DOUBLE_MINUS, DOUBLE_PLUS,
        DOUBLE_LT, DOUBLE_GT, DOUBLE_DOT, TRIPLE_DOT, DOUBLE_QUESTION_MARK, QUESTION_MARK_DOT,
        MINUS_ARROW, EQUALS_ARROW, SEMICOLON, COMPARE, DOUBLE_ASTERISK_EQUALS, DOUBLE_LT_EQUALS,
        DOUBLE_GT_EQUALS,

        // ----- Miscellaneous ----- //
        ID, BOOLEAN_LITERAL, INTEGER_LITERAL, HEX_LITERAL, OCTAL_LITERAL, BINARY_LITERAL,
        FLOAT_LITERAL, STRING_LITERAL, NIL_LITERAL, EOF_TOKEN, NL,

        // ----- Meta ----- //
        META_NORETURN, META_STACKALLOC, META_SRCLOCK
    };

    struct SourceMeta {
        String file;
        int line;
        int col;
    };

    struct Token {
        TokenType type;
        String data;
        SourceMeta meta;
    };

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

    class InvalidInputException: public std::exception {
        String message;
    public:
        InvalidInputException(): message("Invalid input") {}
        InvalidInputException(const SourceMeta& meta, int input) {
            StringBuffer sb;
            sb << "Invalid input '" << (char) input << "' in module ";
            sb << meta.file << " at " << meta.line << ":" << meta.col;
            message = sb.str();
        }

        virtual const char* what() const noexcept {
            return message.c_str();
        }
    };

    class Lexer {
        String file;
        StringBuffer& buf;
        unsigned pos;
        int line;
        int col;
    public:
        Lexer(const String& file, StringBuffer& buf): file(file), buf(buf), pos(0), line(1), col(1) {}

        SourceMeta getSourceMeta() {
            return { file, line, col };
        }
        
        int get() {
            return buf.peek();
        }

        int advance() {
            col++;
            return buf.get();
        }

        void retract(char c) {
            col--;
            buf.putback(c);
        }

        Token lexSymbol() {
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
            
            return { type, content, sourceMeta };
        }

        Token lexNumber() {

        }

        Token lexIdentifier() {
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

            return { type, content, sourceMeta };
        }

        Token lexString(int delimiter) {
            auto sourceMeta = getSourceMeta();
            StringBuffer sb;
            int c;
            while ((c = get()) != delimiter) {
                if (c == '\\') {
                    
                }
                sb << (char) c;
            }
            advance();
            String content = sb.str();
            return { TokenType::STRING_LITERAL, content, sourceMeta };
        }

        Token lexMeta() {

        }

        Token lex() {
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
    };
}