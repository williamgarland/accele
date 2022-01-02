#include "resolver.hpp"

#include "exceptions.hpp"
#include "invariant_types.hpp"
#include "type_builder.hpp"

namespace acl {
Resolver::Resolver(Ast* ast) : ast(ast), currentScope(nullptr) {}

void Resolver::pushScope(Scope* newScope) {
	scopeStack.push_back(currentScope);
	currentScope = newScope;
}

void Resolver::popScope() {
	if (isFunctionScope(currentScope)) {
		List<Symbol*> newSymbols;
		for (auto& symbol : currentScope->symbols) {
			if (dynamic_cast<Parameter*>(symbol)) {
				newSymbols.push_back(symbol);
			}
		}
		currentScope->symbols.clear();
		for (auto& ns : newSymbols) currentScope->symbols.push_back(ns);
	}
	currentScope = scopeStack.back();
	scopeStack.pop_back();
}

void Resolver::resolve() {
	while (ast->stage != ResolutionStage::RESOLVED_ALL) {
		resolveGlobalScope(ast->globalScope);
		if (ast->stage == ResolutionStage::INITIAL_PASS)
			ast->stage = ResolutionStage::RESOLVED_TYPES;
		else if (ast->stage == ResolutionStage::RESOLVED_TYPES)
			ast->stage = ResolutionStage::RESOLVED_ALL;
	}
}

void Resolver::resolveGlobalScope(GlobalScope* scope) {
	pushScope(scope);

	for (auto& content : scope->content) {
		resolveNonLocalContent(content);
	}

	popScope();
}

void Resolver::resolveNonLocalContent(Node* content) {
	if (Variable* variable = dynamic_cast<Variable*>(content))
		resolveVariable(variable);
	else if (Function* function = dynamic_cast<Function*>(content))
		resolveFunctionScope(function);
	else if (Class* c = dynamic_cast<Class*>(content))
		resolveClass(c);
	else if (Struct* s = dynamic_cast<Struct*>(content))
		resolveStruct(s);
	else if (Template* t = dynamic_cast<Template*>(content))
		resolveTemplate(t);
	else if (Enum* e = dynamic_cast<Enum*>(content))
		resolveEnum(e);
	else if (Namespace* n = dynamic_cast<Namespace*>(content))
		resolveNamespace(n);
	else if (Constructor* c = dynamic_cast<Constructor*>(content))
		resolveFunctionScope(c);
	else if (Destructor* d = dynamic_cast<Destructor*>(content))
		resolveFunctionScope(d);
	else if (Import* i = dynamic_cast<Import*>(content))
		resolveImport(i);
	else
		throw AclException(ASP_CORE_UNKNOWN, content->sourceMeta,
						   "Unknown symbol type");
}

void Resolver::resolveGenericType(GenericType* type) {
	if (type->declaredParentType) resolveTypeRef(type->declaredParentType);
}

void Resolver::resolveAlias(Alias* alias) {
	for (auto& g : alias->generics) resolveGenericType(g);
	resolveTypeRef(alias->value);
}

void Resolver::resolveClass(Class* c) {
	pushScope(c);
	for (auto& g : c->generics) resolveGenericType(g);
	if (c->declaredParentType) resolveTypeRef(c->declaredParentType);
	for (auto& t : c->usedTemplates) resolveTypeRef(t);
	for (auto& content : c->content) resolveNonLocalContent(content);
	popScope();
}

void Resolver::resolveStruct(Struct* s) {
	pushScope(s);
	for (auto& g : s->generics) resolveGenericType(g);
	if (s->declaredParentType) resolveTypeRef(s->declaredParentType);
	for (auto& t : s->usedTemplates) resolveTypeRef(t);
	for (auto& content : s->content) resolveNonLocalContent(content);
	popScope();
}

void Resolver::resolveTemplate(Template* t) {
	pushScope(t);
	for (auto& g : t->generics) resolveGenericType(g);
	for (auto& p : t->declaredParentTypes) resolveTypeRef(p);
	for (auto& content : t->content) resolveNonLocalContent(content);
	popScope();
}

void Resolver::resolveEnum(Enum* e) {
	pushScope(e);
	for (auto& g : e->generics) resolveGenericType(g);
	for (auto& t : e->usedTemplates) resolveTypeRef(t);
	for (auto& content : e->content) resolveNonLocalContent(content);
	popScope();
}

void Resolver::resolveNamespace(Namespace* n) {
	pushScope(n);
	for (auto& g : n->generics) resolveGenericType(g);
	for (auto& content : n->content) resolveNonLocalContent(content);
	popScope();
}

void Resolver::resolveFunctionScope(LambdaExpression* scope) {
	pushScope(scope);
	for (auto& param : scope->parameters) resolveParameter(param);
	for (auto& content : scope->content) resolveLocalContent(content);
	popScope();
}

void Resolver::resolveFunctionScope(FunctionBlock* scope) {
	pushScope(scope);
	for (auto& content : scope->content) resolveLocalContent(content);
	popScope();
}

void Resolver::resolveFunctionScope(Function* scope) {
	pushScope(scope);
	for (auto& g : scope->generics) resolveGenericType(g);
	for (auto& param : scope->parameters) resolveParameter(param);
	if (scope->declaredReturnType) {
		resolveTypeRef(scope->declaredReturnType);
		scope->actualReturnType = scope->declaredReturnType;
	} else {
		// TODO: Calculate the actual return type
	}
	for (auto& content : scope->content) resolveLocalContent(content);
	popScope();
}

void Resolver::resolveFunctionScope(SetBlock* scope) {
	pushScope(scope);
	if (scope->parameter) resolveParameter(scope->parameter);
	for (auto& content : scope->content) resolveLocalContent(content);
	popScope();
}

void Resolver::resolveFunctionScope(Constructor* scope) {
	pushScope(scope);
	for (auto& param : scope->parameters) resolveParameter(param);
	for (auto& content : scope->content) resolveLocalContent(content);
	popScope();
}

void Resolver::resolveFunctionScope(Destructor* scope) {
	pushScope(scope);
	for (auto& content : scope->content) resolveLocalContent(content);
	popScope();
}

void Resolver::resolveLocalContent(Node* content) {
	if (IfBlock* n = dynamic_cast<IfBlock*>(content)) {
		resolveExpression(n->condition);
		resolveFunctionScope(n->block);
		for (auto& e : n->elifBlocks) {
			resolveExpression(e->condition);
			resolveFunctionScope(e->block);
		}
		if (n->elseBlock) resolveFunctionScope(n->elseBlock);
	} else if (WhileBlock* n = dynamic_cast<WhileBlock*>(content)) {
		resolveExpression(n->condition);
		resolveFunctionScope(n->block);
	} else if (RepeatBlock* n = dynamic_cast<RepeatBlock*>(content)) {
		resolveExpression(n->condition);
		resolveFunctionScope(n->block);
	} else if (ForBlock* n = dynamic_cast<ForBlock*>(content)) {
		resolveParameter(n->iterator);
		resolveExpression(n->iteratee);
		resolveFunctionScope(n->block);
	} else if (SwitchBlock* n = dynamic_cast<SwitchBlock*>(content)) {
		resolveExpression(n->condition);
		for (auto& c : n->cases) {
			if (c->condition) resolveExpression(c->condition);
			resolveFunctionScope(c->block);
		}
	} else if (TryBlock* n = dynamic_cast<TryBlock*>(content)) {
		resolveFunctionScope(n->block);
		for (auto& c : n->catchBlocks) {
			resolveParameter(c->exceptionVariable);
			resolveFunctionScope(c->block);
		}
	} else if (Variable* n = dynamic_cast<Variable*>(content)) {
		resolveVariable(n);
		currentScope->addSymbol(n);
	} else if (ThrowStatement* n = dynamic_cast<ThrowStatement*>(content)) {
		resolveExpression(n->value);
	} else if (ReturnStatement* n = dynamic_cast<ReturnStatement*>(content)) {
		if (n->value) resolveExpression(n->value);
	} else if (Expression* n = dynamic_cast<Expression*>(content)) {
		resolveExpression(n);
	} else if (FunctionBlock* n = dynamic_cast<FunctionBlock*>(content)) {
		resolveFunctionScope(n);
	} else if (Alias* n = dynamic_cast<Alias*>(content)) {
		resolveAlias(n);
		currentScope->addSymbol(n);
	} else if (WarningMetaDeclaration* n =
				   dynamic_cast<WarningMetaDeclaration*>(content)) {
		resolveLocalContent(n->target);
	} else if (!dynamic_cast<SingleTokenStatement*>(content))
		throw AclException(ASP_CORE_UNKNOWN, content->sourceMeta,
						   "Invalid local content");
}

void Resolver::resolveParameter(Parameter* parameter) {
	if (parameter->declaredType) resolveTypeRef(parameter->declaredType);
}

void Resolver::resolveVariable(Variable* variable) {
	if (variable->declaredType) resolveTypeRef(variable->declaredType);
	if (variable->value) {
		if (Expression* expr = dynamic_cast<Expression*>(variable->value)) {
			resolveExpression(expr);
			if (ast->stage == ResolutionStage::RESOLVED_TYPES) {
				variable->actualType = expr->valueType;
			}
		} else {
			VariableBlock* block = (VariableBlock*)variable->value;
			if (block->getBlock) resolveFunctionScope(block->getBlock);
			if (block->setBlock) resolveFunctionScope(block->setBlock);
			if (block->initBlock) resolveFunctionScope(block->initBlock);
		}
	}
}

void Resolver::resolveTypeRef(TypeRef* typeRef) {
	if (SimpleTypeRef* tr = dynamic_cast<SimpleTypeRef*>(typeRef)) {
		resolveSimpleTypeRef(tr);
	} else if (SuffixTypeRef* tr = dynamic_cast<SuffixTypeRef*>(typeRef)) {
		resolveTypeRef(tr->type);
	} else if (TupleTypeRef* tr = dynamic_cast<TupleTypeRef*>(typeRef)) {
		for (auto& e : tr->elementTypes) resolveTypeRef(e);
	} else if (MapTypeRef* tr = dynamic_cast<MapTypeRef*>(typeRef)) {
		resolveTypeRef(tr->keyType);
		resolveTypeRef(tr->valueType);
	} else if (ArrayTypeRef* tr = dynamic_cast<ArrayTypeRef*>(typeRef)) {
		resolveTypeRef(tr->elementType);
	} else if (FunctionTypeRef* tr = dynamic_cast<FunctionTypeRef*>(typeRef)) {
		for (auto& e : tr->paramTypes) resolveTypeRef(e);
		resolveTypeRef(tr->returnType);
	} else
		throw AclException(ASP_CORE_UNKNOWN, typeRef->sourceMeta,
						   "Unknown type ref type");
}

void Resolver::resolveImport(Import* i) {
	if (i->referent) return;
	// TODO: Make sure the import has a referent by this point.
	// If it doesn't, compile the import target up to the initial AST generation
	// and then set the referent.
}

void Resolver::resolveSimpleTypeRef(SimpleTypeRef* typeRef) {
	auto oldScope = currentScope;
	bool recursiveSearch = true;

	if (typeRef->parent) {
		resolveSimpleTypeRef(typeRef->parent);
		pushScope(dynamic_cast<Scope*>(typeRef->parent->referent));
		recursiveSearch = false;
	}

	if (typeRef->id->type == TokenType::GLOBAL) {
		typeRef->referent = ast->globalScope;
	} else {
		try {
			for (auto& g : typeRef->generics) {
				resolveTypeRef(g);
			}
			List<Symbol*> destSymbols;
			currentScope->resolveSymbol(
				destSymbols, typeRef->id, nullptr, typeRef->generics,
				ResolveFlag::TARGET_TYPE | ResolveFlag::RECURSIVE |
					ResolveFlag::REQUIRE_EXACT_MATCH | ResolveFlag::LEXICAL |
					ResolveFlag::TYPE_HIERARCHY);
			typeRef->referent = destSymbols[0];
		} catch (AclException& e) {
			if (GlobalScope* gs = dynamic_cast<GlobalScope*>(currentScope)) {
				Import* imp = nullptr;
				try {
					imp = gs->resolveImport(typeRef->id);
				} catch (AclException& e2) {
					throw e;
				}
				resolveImport(imp);
				typeRef->referent = imp->referent;
			} else
				throw e;
		}
	}

	if (typeRef->parent) {
		popScope();
	}
}

void Resolver::resolveExpression(Expression* expression,
								 int symbolSearchFlags) {
	if (BinaryExpression* e = dynamic_cast<BinaryExpression*>(expression)) {
		resolveBinaryExpression(e, symbolSearchFlags);
	} else if (UnaryPrefixExpression* e =
				   dynamic_cast<UnaryPrefixExpression*>(expression)) {
		resolvePrefixExpression(e);
	} else if (UnaryPostfixExpression* e =
				   dynamic_cast<UnaryPostfixExpression*>(expression)) {
		resolvePostfixExpression(e);
	} else if (TernaryExpression* e =
				   dynamic_cast<TernaryExpression*>(expression)) {
		resolveTernaryExpression(e);
	} else if (FunctionCallExpression* e =
				   dynamic_cast<FunctionCallExpression*>(expression)) {
		resolveFunctionCallExpression(e);
	} else if (SubscriptExpression* e =
				   dynamic_cast<SubscriptExpression*>(expression)) {
		resolveSubscriptExpression(e);
	} else if (CastingExpression* e =
				   dynamic_cast<CastingExpression*>(expression)) {
		resolveCastingExpression(e);
	} else if (MapLiteralExpression* e =
				   dynamic_cast<MapLiteralExpression*>(expression)) {
		resolveMapLiteralExpression(e);
	} else if (ArrayLiteralExpression* e =
				   dynamic_cast<ArrayLiteralExpression*>(expression)) {
		resolveArrayLiteralExpression(e);
	} else if (TupleLiteralExpression* e =
				   dynamic_cast<TupleLiteralExpression*>(expression)) {
		resolveTupleLiteralExpression(e);
	} else if (LiteralExpression* e =
				   dynamic_cast<LiteralExpression*>(expression)) {
		resolveLiteralExpression(e);
	} else if (IdentifierExpression* e =
				   dynamic_cast<IdentifierExpression*>(expression)) {
		resolveIdentifierExpression(e, symbolSearchFlags);
	} else if (LambdaExpression* e =
				   dynamic_cast<LambdaExpression*>(expression)) {
		resolveLambdaExpression(e);
	} else
		throw AclException(ASP_CORE_UNKNOWN, expression->sourceMeta,
						   "Unknown expression type");
}

void Resolver::resolveBinaryExpression(BinaryExpression* expression,
									   int symbolSearchFlags) {}
void Resolver::resolvePrefixExpression(UnaryPrefixExpression* expression) {}
void Resolver::resolvePostfixExpression(UnaryPostfixExpression* expression) {}

void Resolver::resolveTernaryExpression(TernaryExpression* expression) {
	resolveExpression(expression->arg0);
	resolveExpression(expression->arg1);
	resolveExpression(expression->arg2);

	if (!type::canCastTo(type::getTypeForTypeRef(expression->arg0->valueType),
						 const_cast<bt::InvariantType*>(bt::BOOL)))
		throw AclException(
			ASP_CORE_UNKNOWN, expression->arg0->sourceMeta,
			"Ternary expression condition is not castable to type Bool");

	Type* returnType = type::getMinCommonType(
		type::getTypeForTypeRef(expression->arg1->valueType),
		type::getTypeForTypeRef(expression->arg2->valueType));

	expression->valueType = tb::base(returnType, {}, expression->sourceMeta);
}

void Resolver::resolveFunctionCallExpression(
	FunctionCallExpression* expression) {
	List<TypeRef*> argTypes;
	for (auto& arg : expression->args) {
		resolveExpression(arg);
		argTypes.push_back(arg->valueType);
	}

	IdentifierExpression* dest = nullptr;
	resolveExpressionExpectSymbols(&dest, expression->caller);

	if (dest) {
		auto func = resolveFunctionCallSymbols(expression, argTypes,
											   dest->possibleReferents);
		dest->possibleReferents.clear();
		dest->possibleReferents.push_back(func);
		if (Function* f = dynamic_cast<Function*>(func))
			expression->valueType = f->actualReturnType;
		else {
			Variable* v = dynamic_cast<Variable*>(func);
			expression->valueType =
				dynamic_cast<FunctionTypeRef*>(v->actualType)->returnType;
		}
	} else {
		// Require the caller to be a function type
		auto callerType = expression->caller->valueType;
		if (FunctionTypeRef* f = dynamic_cast<FunctionTypeRef*>(callerType)) {
			expression->valueType = f->returnType;
		} else
			throw AclException(ASP_CORE_UNKNOWN, expression->caller->sourceMeta,
							   "The caller of the function call expression "
							   "does not have a function type");
	}
}

void Resolver::resolveSubscriptExpression(SubscriptExpression* expression) {
	/*resolveExpression(e->target);
	resolveExpression(e->index);
	auto id = Token(TokenType::ID, "[]", e->sourceMeta);
	auto func = resolveFunction(e->target->valueType, &id, TokenType::INFIX,
								{e->index->valueType});
	e->valueType = func->actualReturnType;*/
}

void Resolver::resolveCastingExpression(CastingExpression* expression) {}
void Resolver::resolveMapLiteralExpression(MapLiteralExpression* expression) {}
void Resolver::resolveArrayLiteralExpression(
	ArrayLiteralExpression* expression) {}
void Resolver::resolveTupleLiteralExpression(
	TupleLiteralExpression* expression) {}

void Resolver::resolveLiteralExpression(LiteralExpression* expression) {
	if (expression->value->type == TokenType::STRING_LITERAL)
		expression->valueType =
			tb::base(const_cast<bt::InvariantType*>(bt::STRING), {},
					 bt::STRING->sourceMeta);
	else
		throw "UNIMPLEMENTED";
}

void Resolver::resolveIdentifierExpression(IdentifierExpression* expression,
										   int symbolSearchFlags) {}
void Resolver::resolveLambdaExpression(LambdaExpression* expression) {}

void Resolver::resolveExpressionExpectSymbols(IdentifierExpression** dest,
											  Expression* expression,
											  int symbolSearchFlags) {
	if (BinaryExpression* e = dynamic_cast<BinaryExpression*>(expression)) {
		resolveBinaryExpressionExpectSymbols(dest, e, symbolSearchFlags);
	} else if (UnaryPrefixExpression* e =
				   dynamic_cast<UnaryPrefixExpression*>(expression)) {
		resolvePrefixExpression(e);
	} else if (UnaryPostfixExpression* e =
				   dynamic_cast<UnaryPostfixExpression*>(expression)) {
		resolvePostfixExpression(e);
	} else if (TernaryExpression* e =
				   dynamic_cast<TernaryExpression*>(expression)) {
		resolveTernaryExpression(e);
	} else if (CastingExpression* e =
				   dynamic_cast<CastingExpression*>(expression)) {
		resolveCastingExpression(e);
	} else if (SubscriptExpression* e =
				   dynamic_cast<SubscriptExpression*>(expression)) {
		resolveSubscriptExpression(e);
	} else if (FunctionCallExpression* e =
				   dynamic_cast<FunctionCallExpression*>(expression)) {
		resolveFunctionCallExpression(e);
	} else if (MapLiteralExpression* e =
				   dynamic_cast<MapLiteralExpression*>(expression)) {
		resolveMapLiteralExpression(e);
	} else if (ArrayLiteralExpression* e =
				   dynamic_cast<ArrayLiteralExpression*>(expression)) {
		resolveArrayLiteralExpression(e);
	} else if (TupleLiteralExpression* e =
				   dynamic_cast<TupleLiteralExpression*>(expression)) {
		resolveTupleLiteralExpression(e);
	} else if (LiteralExpression* e =
				   dynamic_cast<LiteralExpression*>(expression)) {
		resolveLiteralExpression(e);
	} else if (LambdaExpression* e =
				   dynamic_cast<LambdaExpression*>(expression)) {
		resolveLambdaExpression(e);
	} else if (IdentifierExpression* e =
				   dynamic_cast<IdentifierExpression*>(expression)) {
		// This is where symbols can come from
		resolveIdentifierExpressionExpectSymbols(dest, e, symbolSearchFlags);
	} else
		throw AclException(ASP_CORE_UNKNOWN, expression->sourceMeta,
						   "Unknown expression type");
}

void Resolver::resolveBinaryExpressionExpectSymbols(
	IdentifierExpression** dest, BinaryExpression* expression,
	int symbolSearchFlags) {
	// Both the '.' and '?.' operators are access operators and therefore affect
	// their respective RHS symbol search flags
	if (expression->op->type == TokenType::DOT) {
		IdentifierExpression* lhs = nullptr;
		resolveExpressionExpectSymbols(&lhs, expression->left);
		int lexicalFlag = 0;
		int typeHierarchyFlag = ResolveFlag::TYPE_HIERARCHY;
		if (lhs) {
			pushScope(getScopeForIdentifierExpression(lhs));

			if (dynamic_cast<Import*>(lhs->possibleReferents[0]) ||
				dynamic_cast<Namespace*>(lhs->possibleReferents[0]) ||
				dynamic_cast<Type*>(lhs->possibleReferents[0])) {
				lexicalFlag = ResolveFlag::LEXICAL;
				typeHierarchyFlag = 0;
			} else
				typeHierarchyFlag = ResolveFlag::TYPE_HIERARCHY;
		} else
			pushScope(getScopeForTypeRef(expression->left->valueType));

		resolveExpressionExpectSymbols(dest, expression->right,
									   lexicalFlag | typeHierarchyFlag);

		popScope();

		expression->valueType = expression->right->valueType;
	} else if (expression->op->type == TokenType::QUESTION_MARK_DOT) {
		// Require LHS to be an optional type
		resolveExpression(expression->left);
		if (SuffixTypeRef* s =
				dynamic_cast<SuffixTypeRef*>(expression->left->valueType)) {
			if (s->suffixSymbol->type != TokenType::QUESTION_MARK &&
				s->suffixSymbol->type != TokenType::EXCLAMATION_POINT)
				throw AclException(ASP_CORE_UNKNOWN,
								   expression->left->sourceMeta,
								   "Expected optional type for left-hand "
								   "argument to optional access expression");
			pushScope(getScopeForTypeRef(s->type));
		} else
			throw AclException(ASP_CORE_UNKNOWN, expression->left->sourceMeta,
							   "Expected optional type for left-hand argument "
							   "to optional access expression");

		resolveExpressionExpectSymbols(dest, expression->right,
									   ResolveFlag::TYPE_HIERARCHY);

		expression->valueType = tb::optional(expression->right->valueType);

		popScope();
	}

	// TODO: Handle the rest of the operators
}

void Resolver::resolveIdentifierExpressionExpectSymbols(
	IdentifierExpression** dest, IdentifierExpression* expression,
	int symbolSearchFlags) {
	*dest = expression;
	for (auto& g : expression->generics) resolveTypeRef(g);
	// We're passing null for the expected type here because we don't know what
	// kind of symbol we're expecting to get
	currentScope->resolveSymbol(
		expression->possibleReferents, expression->value, nullptr,
		expression->generics,
		ResolveFlag::TARGET_TYPE | ResolveFlag::TARGET_VARIABLE |
			ResolveFlag::TARGET_NAMESPACE | symbolSearchFlags);
}

Symbol* Resolver::resolveFunctionCallSymbols(FunctionCallExpression* expression,
											 const List<TypeRef*>& argTypes,
											 const List<Symbol*>& symbols) {
	// We only want symbols that are either variables or functions. If they're
	// variables, then they must have function types.

	// We want the symbol with the best fit for the given argument types. That
	// is, the symbol that supports the argument types and has the lowest
	// overall score.
	Symbol* result = nullptr;
	int minScore = -1;

	for (auto& s : symbols) {
		if (Variable* v = dynamic_cast<Variable*>(s)) {
			if (FunctionTypeRef* f =
					dynamic_cast<FunctionTypeRef*>(v->actualType)) {
				int score = getFunctionTypeScoreForArgs(f, argTypes);
				if ((minScore == -1 && score >= 0) ||
					(score >= 0 && score < minScore)) {
					result = s;
					minScore = score;
				}
			}
		} else if (Function* f = dynamic_cast<Function*>(s)) {
			auto t = getFunctionTypeForFunction(f);

			int score = getFunctionTypeScoreForArgs(t, argTypes);
			if ((minScore == -1 && score >= 0) ||
				(score >= 0 && score < minScore)) {
				result = s;
				minScore = score;
			}

			// We only want to delete the function type - not the param/return
			// types it uses
			t->returnType = nullptr;
			t->paramTypes.clear();

			delete t;
		}
	}

	if (!result)
		throw AclException(ASP_CORE_UNKNOWN, expression->sourceMeta,
						   "Unresolved symbol for function call expression");

	return result;
}

Scope* getScopeForIdentifierExpression(IdentifierExpression* expression) {
	if (expression->possibleReferents.size() != 1)
		throw AclException(ASP_CORE_UNKNOWN, expression->sourceMeta,
						   "Invalid identifier expression");
	if (Scope* scope = dynamic_cast<Scope*>(expression->possibleReferents[0]))
		return scope;
	if (Variable* v =
			dynamic_cast<Variable*>(expression->possibleReferents[0])) {
		return getScopeForTypeRef(v->actualType);
	}
	if (Function* f =
			dynamic_cast<Function*>(expression->possibleReferents[0])) {
		// TODO: return const_cast<bt::InvariantType*>(bt::FUNCTION);
	}

	throw AclException(ASP_CORE_UNKNOWN, expression->sourceMeta,
					   "Invalid identifier expression");
}

Scope* getScopeForTypeRef(TypeRef* t) {
	if (SimpleTypeRef* s = dynamic_cast<SimpleTypeRef*>(t)) {
		if (Scope* scope = dynamic_cast<Scope*>(s->referent)) return scope;
		throw AclException(ASP_CORE_UNKNOWN, t->sourceMeta,
						   "Invalid type reference");
	} else
		return nullptr;
	// TODO: Implement the rest
}

int getFunctionTypeScoreForArgs(FunctionTypeRef* t,
								const List<TypeRef*>& argTypes) {
	int paramIdx = 0;
	int argIdx = 0;
	int score = 0;

	while (true) {
		// If we've reached the limit for both params and args, return score
		// If we've reached the limit for params but not args and the last param
		// isnt variadic, return -1 If we've reached the limit for args but not
		// params and there is more than one param remaining or the last param
		// remaining is not variadic, return -1

		if (paramIdx >= t->paramTypes.size() && argIdx >= argTypes.size()) {
			return score;
		}

		if (paramIdx >= t->paramTypes.size() && argIdx < argTypes.size() &&
			!isVariadicType(t->paramTypes[paramIdx - 1])) {
			return -1;
		}

		if (argIdx >= argTypes.size() && paramIdx < t->paramTypes.size() &&
			(t->paramTypes.size() - paramIdx > 1 ||
			 !isVariadicType(t->paramTypes[paramIdx]))) {
			return -1;
		}

		auto paramType =
			t->paramTypes[paramIdx >= t->paramTypes.size() ? paramIdx - 1
														   : paramIdx];
		auto argType = argTypes[argIdx];

		int typeScore =
			type::getTypeMatchScore(nullptr, type::getTypeForTypeRef(paramType),
									type::getTypeForTypeRef(argType), false);
		if (typeScore == -1) return -1;
		score += typeScore;

		if (paramIdx < t->paramTypes.size()) paramIdx++;
		if (argIdx < argTypes.size()) argIdx++;
	}
}

bool isVariadicType(TypeRef* t) {
	if (SuffixTypeRef* s = dynamic_cast<SuffixTypeRef*>(t)) {
		return s->suffixSymbol->type == TokenType::TRIPLE_DOT;
	}
	return false;
}

FunctionTypeRef* getFunctionTypeForFunction(Function* f) {
	List<TypeRef*> paramTypes;
	for (auto& p : f->parameters)
		paramTypes.push_back(
			p->declaredType ? p->declaredType
							: tb::base(const_cast<bt::InvariantType*>(bt::ANY),
									   {}, p->sourceMeta));
	return tb::function(paramTypes, f->actualReturnType);
}
}  // namespace acl
