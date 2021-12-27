#include "parser.hpp"

#include "exceptions.hpp"

namespace {
const acl::TokenType GLOBAL_FUNCTION_MODIFIERS[] = {
	acl::TokenType::INTERNAL,
	acl::TokenType::UNSAFE,
	acl::TokenType::THROWING,
	acl::TokenType::NOEXCEPT,
	acl::TokenType::ASYNC,
	acl::TokenType::EXTERN,
	acl::TokenType::META_NORETURN,
	acl::TokenType::META_DEPRECATED,
	acl::TokenType::META_ENABLEWARNING,
	acl::TokenType::META_DISABLEWARNING};
}

namespace acl {
Token* Parser::lh(int pos) {
	sync(pos);
	return buffer[current + pos];
}

Token* Parser::match(TokenType type) {
	auto t = lh(0);
	if (t->type != type) throw TokenMismatchException(type, t);
	advance();
	return t;
}

void Parser::matchAndDelete(TokenType type) {
	auto t = match(type);
	if (!isSpeculating()) delete t;
}

void Parser::advance() {
	current++;
	if (current == buffer.size() && !isSpeculating()) {
		current = 0;
		buffer.clear();
	}
	sync(0);
}

void Parser::advanceAndDelete() {
	auto t = lh(0);
	advance();
	if (!isSpeculating()) delete t;
}

int Parser::mark() {
	int result = current;
	marks.push_back(result);
	return result;
}

void Parser::resetToMark() {
	current = marks.back();
	marks.pop_back();
}

void Parser::popMark() { marks.pop_back(); }

bool Parser::isSpeculating() { return !marks.empty(); }

bool Parser::hasNext() {
	return buffer[buffer.size() - 1]->type != TokenType::EOF_TOKEN;
}

void Parser::sync(int pos) {
	if (current + pos > (int)buffer.size() - 1) {
		fill((current + pos) - ((int)buffer.size() - 1));
	}
}

void Parser::fill(int n) {
	for (int i = 0; i < n; i++) {
		buffer.push_back(lexer.nextToken());
	}
}

Parser::Parser(CompilerContext& ctx, Lexer&& lexer)
	: ctx(ctx), lexer(lexer), current(0), currentScope(nullptr) {}

Parser::~Parser() {}

Ast* Parser::parse() {
	sync(0);  // Insert initial token into buffer

	GlobalScope* globalScope = new GlobalScope(lh(0)->meta, {});
	currentScope = globalScope;

	while (hasNext()) {
		auto t = lh(0);
		int current = 0;
		while (isModifier(t->type)) {
			if (t->type == TokenType::META_ENABLEWARNING ||
				t->type == TokenType::META_DISABLEWARNING) {
				// 3 for keyword, lparen, and initial string literal
				current += 3;

				// 2 for comma and next string literal
				while (lh(current)->type == TokenType::COMMA) current += 2;

				t = lh(++current);
			} else
				t = lh(++current);
		}

		if (t->type == TokenType::FUN)
			globalScope->content.push_back(parseGlobalFunction());
		else if (t->type == TokenType::META_ENABLEWARNING ||
				 t->type == TokenType::META_DISABLEWARNING)
			globalScope->content.push_back(parseWarningMeta());
		else if (t->type == TokenType::VAR)
			globalScope->content.push_back(parseGlobalVariable());
		else if (t->type == TokenType::CONST)
			globalScope->content.push_back(parseGlobalConstant());
		else if (t->type == TokenType::ALIAS)
			globalScope->content.push_back(parseGlobalAlias());
		else if (t->type == TokenType::CLASS)
			globalScope->content.push_back(parseGlobalClass());
		else if (t->type == TokenType::STRUCT)
			globalScope->content.push_back(parseGlobalStruct());
		else if (t->type == TokenType::TEMPLATE)
			globalScope->content.push_back(parseGlobalTemplate());
		else if (t->type == TokenType::ENUM)
			globalScope->content.push_back(parseGlobalEnum());
		else if (t->type == TokenType::NAMESPACE)
			globalScope->content.push_back(parseGlobalNamespace());
		else if (t->type == TokenType::IMPORT)
			globalScope->content.push_back(parseImport());
		else if (t->type == TokenType::META_SRCLOCK)
			globalScope->content.push_back(
				parseSourceLock(globalScope->content));
		else if (t->type == TokenType::NL || t->type == TokenType::SEMICOLON)
			advanceAndDelete();
		else
			throw AclException(ASP_GLOBAL_SCOPE, t->meta,
							   "Invalid token in global scope");
	}

	return new Ast(globalScope);
}

Function* Parser::parseGlobalFunction() {
	List<Modifier*> modifiers;
	parseModifiers(GLOBAL_FUNCTION_MODIFIERS, modifiers);
	matchAndDelete(TokenType::FUN);

	// You can't have global operator functions, so we only accept identifiers
	// here
	auto id = match(TokenType::ID);

	List<GenericType*> generics;
	if (lh(0)->type == TokenType::LT) {
		parseGenerics(generics);
	}

	matchAndDelete(TokenType::LPAREN);
	List<Parameter*> parameters;
	parseParameters(parameters);
	matchAndDelete(TokenType::RPAREN);

	TypeRef* declaredReturnType = nullptr;
	if (lh(0)->type == TokenType::MINUS_ARROW) {
		advanceAndDelete();
		declaredReturnType = parseTypeRef();
	}

	Function* function = new Function(modifiers, id, generics, parameters,
									  declaredReturnType, {}, currentScope);
	currentScope->addSymbol(function);
	currentScope = function;

	if (lh(0)->type == TokenType::EQUALS) {
		auto meta = lh(0)->meta;
		advanceAndDelete();
		ReturnStatement* returnStatement =
			new ReturnStatement(meta, parseExpression());
		function->content.push_back(returnStatement);
		parseNewlineEquiv();
	} else if (lh(0)->type == TokenType::LBRACE) {
		advanceAndDelete();
		parseFunctionBlockContent(function->content);
		matchAndDelete(TokenType::RBRACE);
	} else {
		parseNewlineEquiv();
	}

	currentScope = currentScope->parentScope;

	return function;
}

void Parser::parseModifiers(const TokenType types[], List<Modifier*>& dest) {
	auto t = lh(0);
	while (isModifier(t->type)) {
		for (int i = 0; i < sizeof(types) / sizeof(TokenType); i++) {
			if (t->type == types[i]) {
				if (t->type == TokenType::META_ENABLEWARNING ||
					t->type == TokenType::META_DISABLEWARNING) {
					dest.push_back(parseWarningMetaModifier());
				} else {
					dest.push_back(new Modifier{t});
					advance();
				}
				t = lh(0);
				break;
			}
		}
	}
}

WarningMetaDeclaration* Parser::parseWarningMetaModifier() {
	Token* t = lh(0);
	advance();
	matchAndDelete(TokenType::LPAREN);
	List<Token*> args;
	args.push_back(match(TokenType::STRING_LITERAL));
	while (lh(0)->type == TokenType::COMMA) {
		advanceAndDelete();
		args.push_back(match(TokenType::STRING_LITERAL));
	}
	matchAndDelete(TokenType::RPAREN);
	return new WarningMetaDeclaration(t, args, nullptr);
}

void Parser::parseNewlineEquiv(bool greedy) {
	auto t = lh(0);
	if (t->type == TokenType::NL || t->type == TokenType::SEMICOLON) {
		advanceAndDelete();
		while (greedy && lh(0)->type == TokenType::NL ||
			   lh(0)->type == TokenType::SEMICOLON)
			advanceAndDelete();
	} else if (!isNewlineEquivalent(t->type))
		throw AclException(ASP_LINE_TERMINATION, t->meta,
						   "Expected newline or newline-equivalent token");
}

TypeRef* Parser::parseTypeRef() {
	TypeRef* result;
	if (lh(0)->type == TokenType::LPAREN && lh(1)->type == TokenType::RPAREN &&
		lh(1)->type == TokenType::MINUS_ARROW)
		result = parseFunctionTypeRef(nullptr);
	else
		result = parseTypeBase();
	while (isTypeSuffixStart(lh(0)->type)) result = parseTypeSuffix(result);
	return result;
}

TypeRef* Parser::parseTypeBase() {
	auto t = lh(0);
	if (t->type == TokenType::LPAREN) {
		auto meta = t->meta;
		advanceAndDelete();
		List<TypeRef*> elements;
		elements.push_back(parseTypeRef());
		while (lh(0)->type == TokenType::COMMA) {
			advanceAndDelete();
			elements.push_back(parseTypeRef());
		}
		matchAndDelete(TokenType::RPAREN);
		return new TupleTypeRef(meta, elements);
	} else if (t->type == TokenType::LBRACKET) {
		auto meta = t->meta;
		advanceAndDelete();
		TypeRef* key = parseTypeRef();
		matchAndDelete(TokenType::COLON);
		TypeRef* value = parseTypeRef();
		matchAndDelete(TokenType::RBRACKET);
		return new MapTypeRef(meta, key, value);
	} else {
		return parseSimpleTypeBase(nullptr);
	}
}

SimpleTypeRef* Parser::parseSimpleTypeBase(SimpleTypeRef* parent) {
	auto id = match(TokenType::ID);
	List<TypeRef*> generics;
	if (isGenericsStart(lh(0)->type)) parseGenericImpl(generics);

	SimpleTypeRef* result = new SimpleTypeRef(id->meta, id, generics, parent);

	if (lh(0)->type == TokenType::DOT) {
		advanceAndDelete();
		return parseSimpleTypeBase(result);
	}

	return result;
}

TypeRef* Parser::parseTypeSuffix(TypeRef* base) {
	auto t = lh(0);
	if (t->type == TokenType::MINUS_ARROW) return parseFunctionTypeRef(base);
	if (t->type == TokenType::LBRACKET) return parseSubscriptTypeRef(base);
	if (t->type == TokenType::TRIPLE_DOT ||
		t->type == TokenType::EXCLAMATION_POINT ||
		t->type == TokenType::QUESTION_MARK || t->type == TokenType::ASTERISK) {
		advance();
		return new SuffixTypeRef(t->meta, base, t);
	}
	if (t->type == TokenType::DOUBLE_QUESTION_MARK ||
		t->type == TokenType::QUESTION_MARK_DOT ||
		t->type == TokenType::DOUBLE_ASTERISK ||
		t->type == TokenType::ASTERISK_EQUALS ||
		t->type == TokenType::DOUBLE_ASTERISK_EQUALS) {
		auto actualSuffix = relex();
		advance();
		return new SuffixTypeRef(actualSuffix->meta, base, actualSuffix);
	}
	throw AclException(ASP_CORE_UNKNOWN, t->meta,
					   "Invalid type reference suffix");
}

FunctionTypeRef* Parser::parseFunctionTypeRef(TypeRef* parameters) {
	List<TypeRef*> parameterTypes;
	if (parameters) {
		if (TupleTypeRef* asTuple = dynamic_cast<TupleTypeRef*>(parameters)) {
			for (auto& t : asTuple->elementTypes) parameterTypes.push_back(t);
			asTuple->deleteElementTypes = false;
			delete parameters;
		} else {
			parameterTypes.push_back(parameters);
		}
	} else {
		matchAndDelete(TokenType::LPAREN);
		matchAndDelete(TokenType::RPAREN);
	}
	auto meta = lh(0)->meta;
	matchAndDelete(TokenType::MINUS_ARROW);
	TypeRef* returnType = parseTypeRef();
	return new FunctionTypeRef(meta, parameterTypes, returnType);
}

TypeRef* Parser::parseSubscriptTypeRef(TypeRef* base) {
	auto meta = lh(0)->meta;
	matchAndDelete(TokenType::LBRACKET);
	TypeRef* keyType = nullptr;
	if (lh(0)->type != TokenType::RBRACKET) keyType = parseTypeRef();
	matchAndDelete(TokenType::RBRACKET);
	if (keyType) return new MapTypeRef(meta, keyType, base);
	return new ArrayTypeRef(meta, base);
}

// ---------- Global Functions ---------- //
bool isModifier(TokenType type) {
	return type == TokenType::INTERNAL || type == TokenType::PUBLIC ||
		   type == TokenType::PRIVATE || type == TokenType::PROTECTED ||
		   type == TokenType::STATIC || type == TokenType::UNSAFE ||
		   type == TokenType::ATOMIC || type == TokenType::REF ||
		   type == TokenType::STRONG || type == TokenType::WEAK ||
		   type == TokenType::GREEDY || type == TokenType::FINAL ||
		   type == TokenType::OVERRIDE || type == TokenType::INFIX ||
		   type == TokenType::PREFIX || type == TokenType::POSTFIX ||
		   type == TokenType::THROWING || type == TokenType::NOEXCEPT ||
		   type == TokenType::ASYNC || type == TokenType::EXTERN ||
		   type == TokenType::META_DEPRECATED ||
		   type == TokenType::META_EXTERNALINIT ||
		   type == TokenType::META_STACKALLOC ||
		   type == TokenType::META_LAXTHROW ||
		   type == TokenType::META_NORETURN ||
		   type == TokenType::META_ENABLEWARNING ||
		   type == TokenType::META_DISABLEWARNING;
}

bool isNewlineEquivalent(TokenType type) {
	return type == TokenType::RBRACE || type == TokenType::RBRACKET ||
		   type == TokenType::RPAREN || type == TokenType::COMMA ||
		   type == TokenType::EOF_TOKEN;
}

bool isTypeSuffixStart(TokenType type) {
	return type == TokenType::LBRACKET || type == TokenType::TRIPLE_DOT ||
		   type == TokenType::MINUS_ARROW || type == TokenType::QUESTION_MARK ||
		   type == TokenType::DOUBLE_QUESTION_MARK ||
		   type == TokenType::QUESTION_MARK_DOT ||
		   type == TokenType::EXCLAMATION_POINT ||
		   type == TokenType::ASTERISK || type == TokenType::DOUBLE_ASTERISK ||
		   type == TokenType::ASTERISK_EQUALS ||
		   type == TokenType::DOUBLE_ASTERISK_EQUALS;
}
}  // namespace acl