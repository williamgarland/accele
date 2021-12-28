#include "parser.hpp"

#include "exceptions.hpp"

namespace {
constexpr int GLOBAL_FUNCTION_MODIFIERS_LEN = 10;
const acl::TokenType GLOBAL_FUNCTION_MODIFIERS[GLOBAL_FUNCTION_MODIFIERS_LEN] =
	{acl::TokenType::INTERNAL,
	 acl::TokenType::UNSAFE,
	 acl::TokenType::THROWING,
	 acl::TokenType::NOEXCEPT,
	 acl::TokenType::ASYNC,
	 acl::TokenType::EXTERN,
	 acl::TokenType::META_NORETURN,
	 acl::TokenType::META_DEPRECATED,
	 acl::TokenType::META_ENABLEWARNING,
	 acl::TokenType::META_DISABLEWARNING};
};	// namespace

constexpr int LAMBDA_MODIFIERS_LEN = 2;
const acl::TokenType LAMBDA_MODIFIERS[LAMBDA_MODIFIERS_LEN] = {
	acl::TokenType::ASYNC, acl::TokenType::UNSAFE};

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
	if (isSpeculating())
		queuedDeletedTokens.back().push_back(t);
	else
		delete t;
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
	if (isSpeculating())
		queuedDeletedTokens.back().push_back(t);
	else
		delete t;
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

void Parser::popMark(bool deleteQueuedTokens) {
	marks.pop_back();
	auto& queued = queuedDeletedTokens.back();
	if (deleteQueuedTokens) {
		for (auto& t : queued) delete t;
	}
	queuedDeletedTokens.pop_back();
}

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

Token* Parser::relex() {
	return nullptr;	 // TODO: Implement this
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
	parseModifiers(GLOBAL_FUNCTION_MODIFIERS, GLOBAL_FUNCTION_MODIFIERS_LEN,
				   modifiers);
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

void Parser::parseModifiers(const TokenType* types, int typesLen,
							List<Modifier*>& dest) {
	auto t = lh(0);
	while (isModifier(t->type)) {
		bool foundModifier = false;
		for (int i = 0; i < typesLen; i++) {
			if (t->type == types[i]) {
				if (t->type == TokenType::META_ENABLEWARNING ||
					t->type == TokenType::META_DISABLEWARNING) {
					dest.push_back(parseWarningMetaModifier());
				} else {
					dest.push_back(new Modifier{t});
					advance();
				}
				t = lh(0);
				foundModifier = true;
				break;
			}
		}
		if (!foundModifier)
			throw AclException(ASP_CORE_UNKNOWN, t->meta, "Invalid modifier");
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

void Parser::skipNewlines(bool includeSemicolons) {
	while (lh(0)->type == TokenType::NL ||
		   (includeSemicolons && lh(0)->type == TokenType::SEMICOLON))
		advanceAndDelete();
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

Expression* Parser::parseExpression() { return parseAssignmentExpression(); }

Expression* Parser::parseAssignmentExpression() {
	auto left = parseL2Expression();
	if (isAssignmentOperator(lh(0)->type)) {
		auto op = lh(0);
		advance();
		auto right = parseAssignmentExpression();
		return new BinaryExpression(op->meta, op, left, right);
	}
	return left;
}

Expression* Parser::parseL2Expression() {
	mark();
	try {
		auto result = parseLambdaExpression();
		popMark(true);
		return result;
	} catch (AclException& e) {
		resetToMark();
		return parseTernaryExpression();
	}
}

Expression* Parser::parseLambdaExpression() {
	List<Modifier*> modifiers;
	parseModifiers(LAMBDA_MODIFIERS, LAMBDA_MODIFIERS_LEN, modifiers);
	List<Parameter*> parameters;
	parseLambdaParameters(parameters);
	auto meta = lh(0)->meta;
	matchAndDelete(TokenType::EQUALS_ARROW);
	List<Node*> content;
	auto result = new LambdaExpression(meta, modifiers, parameters, content,
									   currentScope);
	currentScope = result;
	parseLambdaBody(content);
	currentScope = currentScope->parentScope;
	return result;
}

void Parser::parseLambdaParameters(List<Parameter*>& dest) {
	if (lh(0)->type == TokenType::LPAREN) {
		advanceAndDelete();
		if (lh(0)->type != TokenType::RPAREN) {
			parseParameters(dest);
		}
		matchAndDelete(TokenType::RPAREN);
	} else {
		dest.push_back(parseParameter());
	}
}

void Parser::parseLambdaBody(List<Node*>& dest) {
	skipNewlines();
	if (lh(0)->type == TokenType::LBRACE) {
		advanceAndDelete();
		parseFunctionBlockContent(dest);
		matchAndDelete(TokenType::RBRACE);
	} else {
		auto meta = lh(0)->meta;
		dest.push_back(new ReturnStatement(meta, parseExpression()));
	}
}

Expression* Parser::parseTernaryExpression() {
	auto arg0 = parseLogicalOrExpression();
	if (lh(0)->type == TokenType::QUESTION_MARK) {
		auto meta = lh(0)->meta;
		advanceAndDelete();
		auto arg1 = parseExpression();
		matchAndDelete(TokenType::COLON);
		auto arg2 = parseExpression();
		return new TernaryExpression(meta, arg0, arg1, arg2);
	}
	return arg0;
}

Expression* Parser::parseLogicalOrExpression() {
	auto left = parseLogicalAndExpression();
	while (lh(0)->type == TokenType::DOUBLE_PIPE ||
		   lh(0)->type == TokenType::OR) {
		auto op = lh(0);
		advance();
		left = new BinaryExpression(op->meta, op, left,
									parseLogicalAndExpression());
	}
	return left;
}

Expression* Parser::parseLogicalAndExpression() {
	auto left = parseBitwiseOrExpression();
	while (lh(0)->type == TokenType::DOUBLE_AMPERSAND ||
		   lh(0)->type == TokenType::AND) {
		auto op = lh(0);
		advance();
		left = new BinaryExpression(op->meta, op, left,
									parseBitwiseOrExpression());
	}
	return left;
}

Expression* Parser::parseBitwiseOrExpression() {
	auto left = parseBitwiseXorExpression();
	while (lh(0)->type == TokenType::PIPE) {
		auto op = lh(0);
		advance();
		left = new BinaryExpression(op->meta, op, left,
									parseBitwiseXorExpression());
	}
	return left;
}

Expression* Parser::parseBitwiseXorExpression() {
	auto left = parseBitwiseAndExpression();
	while (lh(0)->type == TokenType::CARET) {
		auto op = lh(0);
		advance();
		left = new BinaryExpression(op->meta, op, left,
									parseBitwiseAndExpression());
	}
	return left;
}

Expression* Parser::parseBitwiseAndExpression() {
	auto left = parseEqualityExpression();
	while (lh(0)->type == TokenType::AMPERSAND) {
		auto op = lh(0);
		advance();
		left =
			new BinaryExpression(op->meta, op, left, parseEqualityExpression());
	}
	return left;
}

Expression* Parser::parseEqualityExpression() {
	auto left = parseRelationalExpression();
	while (isEqualityOperator(lh(0)->type)) {
		auto op = lh(0);
		advance();
		left = new BinaryExpression(op->meta, op, left,
									parseRelationalExpression());
	}
	return left;
}

Expression* Parser::parseRelationalExpression() {
	auto left = parseNilCoalescingExpression();
	while (isRelationalOperator(lh(0)->type)) {
		auto op = lh(0);
		advance();
		left = new BinaryExpression(op->meta, op, left,
									parseNilCoalescingExpression());
	}
	return left;
}

Expression* Parser::parseNilCoalescingExpression() {
	auto left = parseCastingExpression();
	while (lh(0)->type == TokenType::DOUBLE_QUESTION_MARK) {
		auto op = lh(0);
		advance();
		left =
			new BinaryExpression(op->meta, op, left, parseCastingExpression());
	}
	return left;
}

Expression* Parser::parseCastingExpression() {
	auto left = parseRangeExpression();
	while (isCastingOperator(lh(0)->type)) {
		auto op = lh(0);
		advance();
		left = new CastingExpression(op->meta, op, left, parseTypeRef());
	}
	return left;
}

Expression* Parser::parseRangeExpression() {
	auto left = parseBitshiftExpression();
	while (isRangeOperator(lh(0)->type)) {
		auto op = lh(0);
		advance();
		left =
			new BinaryExpression(op->meta, op, left, parseBitshiftExpression());
	}
	return left;
}

Expression* Parser::parseBitshiftExpression() {
	auto left = parseAdditiveExpression();
	while (isBitshiftOperator(lh(0)->type)) {
		auto op = lh(0);
		advance();
		left =
			new BinaryExpression(op->meta, op, left, parseAdditiveExpression());
	}
	return left;
}

Expression* Parser::parseAdditiveExpression() {
	auto left = parseMultiplicativeExpression();
	while (isAdditiveOperator(lh(0)->type)) {
		auto op = lh(0);
		advance();
		left = new BinaryExpression(op->meta, op, left,
									parseMultiplicativeExpression());
	}
	return left;
}

Expression* Parser::parseMultiplicativeExpression() {
	auto left = parseExponentialExpression();
	while (isMultiplicativeOperator(lh(0)->type)) {
		auto op = lh(0);
		advance();
		left = new BinaryExpression(op->meta, op, left,
									parseExponentialExpression());
	}
	return left;
}

Expression* Parser::parseExponentialExpression() {
	auto left = parsePrefixExpression();
	while (lh(0)->type == TokenType::DOUBLE_ASTERISK) {
		auto op = lh(0);
		advance();
		left =
			new BinaryExpression(op->meta, op, left, parsePrefixExpression());
	}
	return left;
}

Expression* Parser::parsePrefixExpression() {
	List<Token*> operators;
	while (isPrefixOperator(lh(0)->type)) {
		operators.push_back(lh(0));
		advance();
	}
	auto arg = parsePostfixExpression();
	while (operators.size() > 0) {
		auto op = operators.back();
		operators.pop_back();
		arg = new UnaryPrefixExpression(op->meta, op, arg);
	}
	return arg;
}

Expression* Parser::parsePostfixExpression() {
	auto arg = parseAccessCallExpression();
	while (isPostfixOperator(lh(0)->type)) {
		auto op = lh(0);
		advance();
		arg = new UnaryPostfixExpression(op->meta, op, arg);
	}
	return arg;
}

Expression* Parser::parseAccessCallExpression() {
	auto left = parsePrimaryExpression();
	while (isAccessOperator(lh(0)->type) || isCallOperator(lh(0)->type)) {
		if (isAccessOperator(lh(0)->type)) {
			auto op = lh(0);
			advance();
			left = new BinaryExpression(op->meta, op, left,
										parsePrimaryExpression());
		} else
			left = parseCallExpressionEnd(left);
	}
	return left;
}

Expression* Parser::parseCallExpressionEnd(Expression* caller) {
	if (lh(0)->type == TokenType::LPAREN) {
		auto meta = lh(0)->meta;
		advanceAndDelete();
		skipNewlines();
		List<Expression*> args;
		if (lh(0)->type != TokenType::RPAREN) parseExpressionList(args);
		matchAndDelete(TokenType::RPAREN);
		return new FunctionCallExpression(meta, caller, args);
	}

	auto meta = lh(0)->meta;
	matchAndDelete(TokenType::LBRACE);
	auto arg = parseExpression();
	matchAndDelete(TokenType::RBRACE);
	return new SubscriptExpression(meta, caller, arg);
}

Expression* Parser::parsePrimaryExpression() {
	auto token = lh(0);

	if (isLiteral(token->type)) {
		advance();
		return new LiteralExpression(token);
	}
	if (token->type == TokenType::GLOBAL || token->type == TokenType::ID)
		return parseIdentifierExpression();
	if (token->type == TokenType::LBRACKET)
		return parseArrayOrMapLiteralExpression();

	auto meta = token->meta;
	matchAndDelete(TokenType::LPAREN);
	auto expr = parseExpression();
	if (lh(0)->type == TokenType::COMMA) {
		List<Expression*> args;
		args.push_back(expr);
		while (lh(0)->type == TokenType::COMMA) {
			advanceAndDelete();
			args.push_back(parseExpression());
		}
		matchAndDelete(TokenType::RPAREN);
		return new TupleLiteralExpression(meta, args);
	}
	matchAndDelete(TokenType::RPAREN);
	return expr;
}

Expression* Parser::parseIdentifierExpression() {
	bool globalPrefix = false;
	if (lh(0)->type == TokenType::GLOBAL) {
		advanceAndDelete();
		matchAndDelete(TokenType::DOT);
		globalPrefix = true;
	}
	auto id = match(TokenType::ID);
	List<TypeRef*> generics;
	if (lh(0)->type == TokenType::LT) {
		mark();
		try {
			parseGenericImpl(generics);
			popMark();
		} catch (AclException& e) {
			resetToMark();
		}
	}
	return new IdentifierExpression(id, generics, globalPrefix);
}

Expression* Parser::parseArrayOrMapLiteralExpression() {
	auto meta = lh(0)->meta;
	matchAndDelete(TokenType::LBRACKET);
	skipNewlines();
	List<Expression*> args;
	List<Expression*> values;
	bool mapLiteral = false;

	if (lh(0)->type != TokenType::RBRACKET) {
		args.push_back(parseExpression());
		skipNewlines();

		if (lh(0)->type == TokenType::COLON) {
			advanceAndDelete();
			values.push_back(parseExpression());
			mapLiteral = true;
			skipNewlines();
		}
	}

	while (lh(0)->type == TokenType::COMMA) {
		advanceAndDelete();
		skipNewlines();
		args.push_back(parseExpression());
		skipNewlines();

		if (mapLiteral) {
			matchAndDelete(TokenType::COLON);
			skipNewlines();
			values.push_back(parseExpression());
			skipNewlines();
		}
	}

	matchAndDelete(TokenType::RBRACKET);

	if (mapLiteral) return new MapLiteralExpression(meta, args, values);

	return new ArrayLiteralExpression(meta, args);
}

void Parser::parseExpressionList(List<Expression*>& dest) {
	dest.push_back(parseExpression());
	while (lh(0)->type == TokenType::COMMA) {
		advanceAndDelete();
		dest.push_back(parseExpression());
	}
}

WarningMetaDeclaration* Parser::parseWarningMeta() { return nullptr; }
Variable* Parser::parseGlobalVariable() { return nullptr; }
Variable* Parser::parseGlobalConstant() { return nullptr; }
Alias* Parser::parseGlobalAlias() { return nullptr; }
Class* Parser::parseGlobalClass() { return nullptr; }
Struct* Parser::parseGlobalStruct() { return nullptr; }
Template* Parser::parseGlobalTemplate() { return nullptr; }
Enum* Parser::parseGlobalEnum() { return nullptr; }
Namespace* Parser::parseGlobalNamespace() { return nullptr; }
Import* Parser::parseImport() { return nullptr; }
MetaDeclaration* Parser::parseSourceLock(const List<Node*>& globalContent) {
	return nullptr;
}

void Parser::parseParameters(List<Parameter*>& dest) {}
Parameter* Parser::parseParameter() { return nullptr; }
void Parser::parseGenerics(List<GenericType*>& dest) {}

void Parser::parseFunctionBlockContent(List<Node*>& dest) {
	skipNewlines(true);
	while (lh(0)->type != TokenType::RBRACE) {
		auto t = lh(0);
		if (t->type == TokenType::IF) {
		} else if (t->type == TokenType::WHILE) {
		} else if (t->type == TokenType::REPEAT) {
		} else if (t->type == TokenType::FOR) {
		} else if (t->type == TokenType::SWITCH) {
		} else if (t->type == TokenType::LBRACE ||
				   (t->type == TokenType::UNSAFE &&
					lh(1)->type == TokenType::LBRACE)) {
		} else if (t->type == TokenType::BREAK ||
				   t->type == TokenType::CONTINUE ||
				   t->type == TokenType::FALL) {
		} else if (t->type == TokenType::VAR) {
		} else if (t->type == TokenType::CONST) {
		} else if (t->type == TokenType::ALIAS) {
		} else if (isLocalVariableModifier(t->type)) {
		} else if (t->type == TokenType::THROW) {
		} else if (t->type == TokenType::RETURN) {
		} else {
			dest.push_back(parseExpression());
			parseNewlineEquiv();
		}
		skipNewlines(true);
	}
}

void Parser::parseGenericImpl(List<TypeRef*>& dest) {}

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

bool isGenericsStart(TokenType type) {
	return type == TokenType::LT || type == TokenType::DOUBLE_LT ||
		   type == TokenType::DOUBLE_LT_EQUALS || type == TokenType::LT_EQUALS;
}

bool isAssignmentOperator(TokenType type) {
	return type == TokenType::EQUALS || type == TokenType::PIPE_EQUALS ||
		   type == TokenType::PLUS_EQUALS || type == TokenType::CARET_EQUALS ||
		   type == TokenType::MINUS_EQUALS || type == TokenType::SLASH_EQUALS ||
		   type == TokenType::TILDE_EQUALS ||
		   type == TokenType::PERCENT_EQUALS ||
		   type == TokenType::ASTERISK_EQUALS ||
		   type == TokenType::AMPERSAND_EQUALS ||
		   type == TokenType::DOUBLE_GT_EQUALS ||
		   type == TokenType::DOUBLE_LT_EQUALS ||
		   type == TokenType::DOUBLE_ASTERISK_EQUALS;
}

bool isEqualityOperator(TokenType type) {
	return type == TokenType::DOUBLE_EQUALS ||
		   type == TokenType::TRIPLE_EQUALS ||
		   type == TokenType::EXCLAMATION_POINT_EQUALS ||
		   type == TokenType::EXCLAMATION_POINT_DOUBLE_EQUALS;
}

bool isRelationalOperator(TokenType type) {
	return type == TokenType::LT || type == TokenType::GT ||
		   type == TokenType::LT_EQUALS || type == TokenType::GT_EQUALS ||
		   type == TokenType::COMPARE;
}

bool isCastingOperator(TokenType type) {
	return type == TokenType::AS || type == TokenType::AS_OPTIONAL ||
		   type == TokenType::AS_UNWRAPPED || type == TokenType::IS;
}

bool isRangeOperator(TokenType type) {
	return type == TokenType::DOUBLE_DOT || type == TokenType::TRIPLE_DOT;
}

bool isBitshiftOperator(TokenType type) {
	return type == TokenType::DOUBLE_LT || type == TokenType::DOUBLE_GT;
}

bool isAdditiveOperator(TokenType type) {
	return type == TokenType::PLUS || type == TokenType::MINUS;
}

bool isMultiplicativeOperator(TokenType type) {
	return type == TokenType::ASTERISK || type == TokenType::SLASH ||
		   type == TokenType::PERCENT;
}

bool isPrefixOperator(TokenType type) {
	return type == TokenType::PLUS || type == TokenType::MINUS ||
		   type == TokenType::DOUBLE_PLUS || type == TokenType::DOUBLE_MINUS ||
		   type == TokenType::TILDE || type == TokenType::EXCLAMATION_POINT ||
		   type == TokenType::ASTERISK || type == TokenType::AMPERSAND ||
		   type == TokenType::RELEASE || type == TokenType::TRY_OPTIONAL ||
		   type == TokenType::TRY_UNWRAPPED || type == TokenType::AWAIT ||
		   type == TokenType::NOT;
}

bool isPostfixOperator(TokenType type) {
	return type == TokenType::DOUBLE_PLUS || type == TokenType::DOUBLE_MINUS ||
		   type == TokenType::EXCLAMATION_POINT;
}

bool isAccessOperator(TokenType type) {
	return type == TokenType::DOT || type == TokenType::QUESTION_MARK_DOT;
}

bool isCallOperator(TokenType type) {
	return type == TokenType::LPAREN || type == TokenType::LBRACKET;
}

bool isLiteral(TokenType type) {
	return type == TokenType::HEX_LITERAL || type == TokenType::NIL_LITERAL ||
		   type == TokenType::FLOAT_LITERAL ||
		   type == TokenType::OCTAL_LITERAL ||
		   type == TokenType::BINARY_LITERAL ||
		   type == TokenType::STRING_LITERAL ||
		   type == TokenType::BOOLEAN_LITERAL ||
		   type == TokenType::INTEGER_LITERAL || type == TokenType::SELF ||
		   type == TokenType::SUPER;
}

bool isLocalVariableModifier(TokenType type) {
	return type == TokenType::ATOMIC || type == TokenType::REF ||
		   type == TokenType::GREEDY || type == TokenType::STRONG ||
		   type == TokenType::WEAK;
}
}  // namespace acl