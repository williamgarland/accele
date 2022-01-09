#include "resolver.hpp"

#include <algorithm>
#include <utility>

#include "exceptions.hpp"
#include "import_handler.hpp"
#include "invariant_types.hpp"
#include "type_builder.hpp"

namespace {
using namespace acl;

SearchTarget getSearchTarget(const Symbol* s) {
	if (dynamic_cast<const Variable*>(s) || dynamic_cast<const Parameter*>(s) ||
		dynamic_cast<const Function*>(s) || dynamic_cast<const Constructor*>(s))
		return SearchTarget::VARIABLE;
	else if (dynamic_cast<const Type*>(s))
		return SearchTarget::TYPE;
	return SearchTarget::NAMESPACE;
}

resolve::ResultOrigin getResultOrigin(const Scope* owningScope,
									  const Symbol* s) {
	if (isFunctionScope(owningScope)) return resolve::ResultOrigin::LOCAL;
	if (isStaticSymbol(owningScope, s)) return resolve::ResultOrigin::STATIC;
	return resolve::ResultOrigin::TYPE_HIERARCHY;
}

const GlobalScope* getGlobalScope(const Scope* scope) {
	auto currentScope = scope;
	while (currentScope->parentScope) currentScope = currentScope->parentScope;

	return dynamic_cast<const GlobalScope*>(currentScope);
}

static void resolveSymbol0(List<resolve::SearchResult>& dest, Scope* scope,
						   const Token* id, bool recursive, bool allowExternal,
						   const List<SearchTarget>& targets) {
	for (auto& s : scope->symbols) {
		if (s->id->data == id->data &&
			listContains(targets, getSearchTarget(s)))
			dest.push_back({s, scope, getResultOrigin(scope, s)});
	}

	if (Type* t = dynamic_cast<Type*>(scope)) {
		for (auto& parent : t->parentTypes) {
			if (Scope* parentScope = dynamic_cast<Scope*>(parent->actualType))
				resolveSymbol0(dest, parentScope, id, false, false, targets);
		}
	}

	if (recursive && scope->parentScope)
		resolveSymbol0(dest, scope->parentScope, id, true, allowExternal,
					   targets);
	if (allowExternal) {
		if (GlobalScope* gs = dynamic_cast<GlobalScope*>(scope)) {
			for (auto& i : gs->imports) {
				resolveSymbol0(dest, i->referent->globalScope, id, false, false,
							   targets);
			}
		}
	}
}

void resolveSymbol(List<resolve::SearchResult>& dest, Scope* scope,
				   const Token* id, bool recursive, bool allowExternal,
				   const List<SearchTarget>& targets) {
	if (id->type == TokenType::GLOBAL) {
		auto gs = const_cast<GlobalScope*>(getGlobalScope(scope));
		dest.push_back({dynamic_cast<Symbol*>(gs), dynamic_cast<Scope*>(gs),
						resolve::ResultOrigin::STATIC});
		return;
	}

	resolveSymbol0(dest, scope, id, recursive, allowExternal, targets);

	if (recursive && listContains(targets, SearchTarget::TYPE)) {
		try {
			auto s =
				const_cast<bt::InvariantType*>(bt::resolveInvariantType(id));
			dest.push_back({dynamic_cast<Symbol*>(s), dynamic_cast<Scope*>(s),
							resolve::ResultOrigin::STATIC});
		} catch (AclException& e) {
		}
	}
}

struct SymbolCandidateProblem {
	int ec;
	SourceMeta location;
};

void diagnoseVisibility(List<SymbolCandidateProblem>& problems,
						const resolve::SearchResult& candidate,
						const SearchCriteria& searchCriteria,
						const SourceMeta& refererMeta,
						const Scope* lexicalScope) {
	SourceMeta visMeta = refererMeta;
	auto visibility =
		getSymbolVisibility(candidate.owningScope, candidate.symbol,
							searchCriteria.modifiable, visMeta);

	if (visibility == TokenType::INTERNAL) {
		// Assert owning modules match
		const auto& candidateGs = getGlobalScope(candidate.owningScope);
		const auto& lexicalGs = getGlobalScope(lexicalScope);

		if (candidateGs != lexicalGs) {
			problems.push_back({ACL_EC_SYMBOL_NOT_VISIBLE, refererMeta});
		}
	} else if (visibility == TokenType::PROTECTED) {
		// "protected" can only be used on symbols which are declared in a type
		const Type* candidateType =
			dynamic_cast<const Type*>(candidate.owningScope);
		if (!candidateType) {
			// TODO: Print error here
			problems.push_back({ACL_EC_INVALID_MODIFIER, visMeta});
			return;
		}

		// Assert both have same type in lexical scope hierarchy
		const Scope* currentScope = lexicalScope;
		while (currentScope) {
			if (currentScope == candidate.owningScope) return;
			currentScope = currentScope->parentScope;
		}

		// Otherwise, assert the type is a parent of the nearest containing type
		currentScope = lexicalScope;
		const Type* type = nullptr;
		while (currentScope) {
			if (const Type* t = dynamic_cast<const Type*>(currentScope)) {
				type = t;
				break;
			}
			currentScope = currentScope->parentScope;
		}

		auto typeRef = tb::base(const_cast<Type*>(type), {}, type->sourceMeta);
		auto candidateTypeRef = tb::base(const_cast<Type*>(candidateType), {},
										 candidateType->sourceMeta);

		if (!type || !type::canCastTo(typeRef, candidateTypeRef)) {
			problems.push_back({ACL_EC_SYMBOL_NOT_VISIBLE, refererMeta});
		}

		delete typeRef;
		delete candidateTypeRef;
	} else if (visibility == TokenType::PRIVATE) {
		// "private" can only be used on symbols which are declared in a type or
		// namespace
		if (!dynamic_cast<const Type*>(candidate.owningScope) &&
			!dynamic_cast<const Namespace*>(candidate.owningScope)) {
			// TODO: Print error here
			problems.push_back({ACL_EC_INVALID_MODIFIER, visMeta});
			return;
		}

		// Assert both have same scope in lexical scope hierarchy
		const Scope* currentScope = lexicalScope;
		while (currentScope) {
			if (currentScope == candidate.owningScope) return;
			currentScope = currentScope->parentScope;
		}

		problems.push_back({ACL_EC_SYMBOL_NOT_VISIBLE, refererMeta});
	}
}

void diagnoseStaticness(List<SymbolCandidateProblem>& problems,
						const resolve::SearchResult& candidate,
						const SourceMeta& refererMeta) {
	bool isStatic = isStaticSymbol(candidate.owningScope, candidate.symbol);
	if (isStatic && candidate.origin == resolve::ResultOrigin::TYPE_HIERARCHY) {
		problems.push_back({ACL_EC_STATIC_ACCESS_VIA_INSTANCE, refererMeta});
	} else if (!isStatic && candidate.origin == resolve::ResultOrigin::STATIC) {
		problems.push_back({ACL_EC_INSTANCE_ACCESS_VIA_STATIC, refererMeta});
	}
}

void diagnoseGenerics(List<SymbolCandidateProblem>& problems,
					  const resolve::SearchResult& candidate,
					  const List<TypeRef*>& generics,
					  const SearchCriteria& searchCriteria,
					  const SourceMeta& refererMeta) {
	if (const Namespace* n = dynamic_cast<const Namespace*>(candidate.symbol)) {
		if (n->generics.size() < generics.size())
			problems.push_back(
				{ACL_EC_TOO_MANY_GENERICS, generics.back()->sourceMeta});
		else if (n->generics.size() > generics.size())
			problems.push_back({ACL_EC_INSUFFICIENT_GENERICS, refererMeta});
		else {
			for (int i = 0; i < generics.size(); i++) {
				if (!type::genericAcceptsType(n->generics[i], generics[i]))
					problems.push_back(
						{ACL_EC_GENERICS_MISMATCH, generics[i]->sourceMeta});
			}
		}
	} else if (const GenericType* g =
				   dynamic_cast<const GenericType*>(candidate.symbol)) {
		if (!generics.empty()) {
			problems.push_back(
				{ACL_EC_TOO_MANY_GENERICS, generics.back()->sourceMeta});
		}
	} else if (const Type* n = dynamic_cast<const Type*>(candidate.symbol)) {
		if (n->generics.size() < generics.size()) {
			problems.push_back(
				{ACL_EC_TOO_MANY_GENERICS, generics.back()->sourceMeta});
			return;
		} else if (searchCriteria.requireExactMatch &&
				   n->generics.size() > generics.size()) {
			problems.push_back({ACL_EC_INSUFFICIENT_GENERICS, refererMeta});
		}
		for (int i = 0; i < generics.size(); i++) {
			if (!type::genericAcceptsType(n->generics[i], generics[i]))
				problems.push_back(
					{ACL_EC_GENERICS_MISMATCH, generics[i]->sourceMeta});
		}
	} else if (const Function* n =
				   dynamic_cast<const Function*>(candidate.symbol)) {
		if (n->generics.size() < generics.size())
			problems.push_back(
				{ACL_EC_TOO_MANY_GENERICS, generics.back()->sourceMeta});
		for (int i = 0; i < generics.size(); i++) {
			if (!type::genericAcceptsType(n->generics[i], generics[i]))
				problems.push_back(
					{ACL_EC_GENERICS_MISMATCH, generics[i]->sourceMeta});
		}
	} else if (const Constructor* n =
				   dynamic_cast<const Constructor*>(candidate.symbol)) {
		const auto& targetGenerics =
			dynamic_cast<const Type*>(n->parentScope)->generics;
		if (targetGenerics.size() < generics.size()) {
			problems.push_back(
				{ACL_EC_TOO_MANY_GENERICS, generics.back()->sourceMeta});
			return;
		} else if (searchCriteria.requireExactMatch &&
				   targetGenerics.size() > generics.size()) {
			problems.push_back({ACL_EC_INSUFFICIENT_GENERICS, refererMeta});
		}
		for (int i = 0; i < generics.size(); i++) {
			if (!type::genericAcceptsType(targetGenerics[i], generics[i]))
				problems.push_back(
					{ACL_EC_GENERICS_MISMATCH, generics[i]->sourceMeta});
		}
	} else if (!generics.empty()) {
		problems.push_back(
			{ACL_EC_TOO_MANY_GENERICS, generics.back()->sourceMeta});
	}
}

void findSymbolCandidateProblems(const resolve::SearchResult& candidate,
								 const List<TypeRef*>& generics,
								 const SearchCriteria& searchCriteria,
								 const SourceMeta& refererMeta,
								 const Scope* lexicalScope,
								 List<SymbolCandidateProblem>& problems) {
	diagnoseVisibility(problems, candidate, searchCriteria, refererMeta,
					   lexicalScope);
	diagnoseStaticness(problems, candidate, refererMeta);
	diagnoseGenerics(problems, candidate, generics, searchCriteria,
					 refererMeta);
}

Symbol* getSymbolReferent(const List<resolve::SearchResult>& results,
						  const List<TypeRef*>& generics,
						  const SearchCriteria& searchCriteria,
						  const SourceMeta& refererMeta,
						  const Scope* lexicalScope) {
	if (results.empty()) {
		throw AclException(ASP_CORE_UNKNOWN, refererMeta, "Unresolved symbol");
	}

	bool first = true;
	List<SymbolCandidateProblem> initialCandidateProblems;
	for (auto& r : results) {
		List<SymbolCandidateProblem> problems;
		findSymbolCandidateProblems(r, generics, searchCriteria, refererMeta,
									lexicalScope, problems);

		if (problems.empty())
			return r.symbol;
		else if (first) {
			initialCandidateProblems.insert(initialCandidateProblems.end(),
											problems.begin(), problems.end());
		}

		first = false;
	}

	auto initialCandidate = results[0];

	for (const auto& problem : initialCandidateProblems) {
		StringBuffer sb;
		sb << "Error " << problem.ec << " in module " << problem.location.file
		   << " at line " << problem.location.line << ", col "
		   << problem.location.col;
		String s = sb.str();
		log::warn(s);
	}

	return initialCandidate.symbol;
}

bool isStaticContext(Scope* scope) {
	Scope* currentScope = scope;
	while (!dynamic_cast<Type*>(currentScope->parentScope)) {
		if (dynamic_cast<GlobalScope*>(currentScope)) return true;
		currentScope = currentScope->parentScope;
	}

	if (Function* f = dynamic_cast<Function*>(currentScope)) {
		for (auto& m : f->modifiers)
			if (m->content->type == TokenType::STATIC) return true;
		return false;
	}

	return dynamic_cast<FunctionBlock*>(currentScope) ||
		   dynamic_cast<SetBlock*>(currentScope) ||
		   dynamic_cast<Constructor*>(currentScope);
}

int getRequiredArity(const List<TypeRef*>& expected, bool& variadicDest) {
	int result = 0;
	SourceMeta* initialVarargsMeta = nullptr;
	for (auto& t : expected) {
		int amt = 1;
		if (SuffixTypeRef* s = dynamic_cast<SuffixTypeRef*>(t)) {
			if (s->suffixSymbol->type == TokenType::TRIPLE_DOT &&
				initialVarargsMeta)
				throw AclException(
					ASP_CORE_UNKNOWN, *initialVarargsMeta,
					"A variadic parameter must be the final parameter");
			else if (s->suffixSymbol->type == TokenType::TRIPLE_DOT) {
				initialVarargsMeta = &s->sourceMeta;
				amt = 0;
				variadicDest = true;
			}
		}

		result += amt;
	}

	return result;
}

void validateFunctionCallArgs(const List<TypeRef*>& expected,
							  const List<TypeRef*>& actual,
							  const SourceMeta& sourceMeta) {
	bool variadic = false;
	int required = getRequiredArity(expected, variadic);
	if (actual.size() < required)
		throw AclException(
			ASP_CORE_UNKNOWN, sourceMeta,
			"Insufficient number of arguments for function call");

	// TODO: Replace immediate throwing when finding one error with one that
	// shows all argument errors
	for (int i = 0; i < actual.size(); i++) {
		auto argType = actual[i];
		if (i >= expected.size() && !variadic)
			throw AclException(ASP_CORE_UNKNOWN, argType->sourceMeta,
							   "Too many arguments for function call");
		else if (i >= expected.size()) {
			SuffixTypeRef* s =
				dynamic_cast<SuffixTypeRef*>(expected[expected.size() - 1]);
			if (!type::canCastTo(argType, s->type))
				throw AclException(ASP_CORE_UNKNOWN, argType->sourceMeta,
								   "Invalid argument for function call");
		} else if (!type::canCastTo(argType, expected[i]))
			throw AclException(ASP_CORE_UNKNOWN, argType->sourceMeta,
							   "Invalid argument for function call");
	}
}

int getFunctionArgsScore(const List<TypeRef*>& expected,
						 const List<TypeRef*>& actual,
						 const SourceMeta& callerMeta, bool& variadicDest) {
	int result = 0;

	int required = getRequiredArity(expected, variadicDest);
	if (actual.size() < required) return -1;

	for (int i = 0; i < actual.size(); i++) {
		auto argType = actual[i];
		if (i >= expected.size() && !variadicDest)
			return -1;
		else if (i >= expected.size()) {
			SuffixTypeRef* s =
				dynamic_cast<SuffixTypeRef*>(expected[expected.size() - 1]);
			int score =
				type::getTypeMatchScore(nullptr, argType, s->type, false);
			if (score == -1) return -1;
			result += score;
		} else {
			int score =
				type::getTypeMatchScore(nullptr, argType, expected[i], false);
			if (score == -1) return -1;
			result += score;
		}
	}

	return result;
}

struct FCCandidate {
	int score;
	bool variadic;
};

void getFunctionCallCandidateScores(
	const List<List<std::pair<Symbol*, FunctionTypeRef*>>>& candidates,
	const List<TypeRef*>& argTypes, List<List<FCCandidate>>& candidateScores,
	const SourceMeta& callerMeta) {
	for (int i = 0; i < candidates.size(); i++) {
		List<FCCandidate> tmp;
		for (int j = 0; j < candidates[i].size(); j++) {
			bool variadic = false;
			int score =
				getFunctionArgsScore(std::get<1>(candidates[i][j])->paramTypes,
									 argTypes, callerMeta, variadic);
			tmp.push_back({score, variadic});
		}
		candidateScores.push_back(tmp);
	}
}

void sortFccScores(const List<List<FCCandidate>>& scores,
				   List<std::pair<int, int>>& indices) {
	using IntermediateType = std::pair<std::pair<int, int>, const FCCandidate*>;
	List<IntermediateType> variadic;
	List<IntermediateType> nonVariadic;

	for (int i = 0; i < scores.size(); i++) {
		for (int j = 0; j < scores[i].size(); j++) {
			if (scores[i][j].variadic) {
				variadic.push_back(
					std::make_pair(std::make_pair(i, j), &scores[i][j]));
			} else {
				nonVariadic.push_back(
					std::make_pair(std::make_pair(i, j), &scores[i][j]));
			}
		}
	}

	std::sort(nonVariadic.begin(), nonVariadic.end(),
			  [](IntermediateType& a, IntermediateType& b) {
				  return std::get<1>(a)->score < std::get<1>(b)->score;
			  });

	std::sort(variadic.begin(), variadic.end(),
			  [](IntermediateType& a, IntermediateType& b) {
				  return std::get<1>(a)->score < std::get<1>(b)->score;
			  });

	for (auto& e : nonVariadic) {
		indices.push_back(std::get<0>(e));
	}

	for (auto& e : variadic) {
		indices.push_back(std::get<0>(e));
	}
}

resolve::SearchResult getFccSearchResult(const resolve::SearchResult& original,
										 Symbol* symbol) {
	if (Constructor* c = dynamic_cast<Constructor*>(symbol)) {
		return {symbol, c->parentScope, original.origin};
	} else
		return {symbol, original.owningScope, original.origin};
}

Scope* getScopeFromTypeRef(TypeRef* n) {
	auto t = n->actualType;
	if (Alias* a = dynamic_cast<Alias*>(t))
		return getScopeFromTypeRef(a->value);
	return dynamic_cast<Scope*>(t);
}

Scope* getScopeFromExpression(Expression* n) {
	if (IdentifierExpression* id = dynamic_cast<IdentifierExpression*>(n)) {
		auto s = id->referent;
		if (Namespace* ns = dynamic_cast<Namespace*>(s))
			return ns;
		else if (Import* i = dynamic_cast<Import*>(s))
			return i->referent->globalScope;
		else if (Type* t = dynamic_cast<Type*>(s)) {
			if (Alias* a = dynamic_cast<Alias*>(t))
				return getScopeFromTypeRef(a->value);
			if (Scope* scope = dynamic_cast<Scope*>(t)) return scope;
			return getScopeFromTypeRef(id->valueType);
		} else
			return getScopeFromTypeRef(id->valueType);
	} else if (BinaryExpression* b = dynamic_cast<BinaryExpression*>(n)) {
		if (b->op->type == TokenType::DOT ||
			b->op->type == TokenType::QUESTION_MARK_DOT) {
			return getScopeFromExpression(b->right);
		} else
			return getScopeFromTypeRef(n->valueType);
	} else {
		return getScopeFromTypeRef(n->valueType);
	}
}

TypeRef* getIteratorElementType(TypeRef* iteratorType) {
	// TODO: Implement this
	return nullptr;
}

bool isOwningFunctionScope(Scope* scope) {
	if (FunctionBlock* block = dynamic_cast<FunctionBlock*>(scope)) {
		return block->blockType == TokenType::GET ||
			   block->blockType == TokenType::INIT;
	}

	return dynamic_cast<Function*>(scope) ||
		   dynamic_cast<LambdaExpression*>(scope) ||
		   dynamic_cast<Constructor*>(scope) ||
		   dynamic_cast<Destructor*>(scope) || dynamic_cast<SetBlock*>(scope);
}

Scope* getOwningFunction(Scope* currentScope) {
	while (currentScope && !isOwningFunctionScope(currentScope))
		currentScope = currentScope->parentScope;
	return currentScope;
}
}  // namespace

namespace acl {
void Resolver::pushSymbol(Symbol* symbol) { symbolStack.push_back(symbol); }

void Resolver::popSymbol() { symbolStack.pop_back(); }

bool Resolver::stackContainsSymbol(Symbol* symbol) {
	for (const auto& s : symbolStack)
		if (s == symbol) return true;
	return false;
}

void Resolver::pushScope(Scope* scope, bool isLexicalScope) {
	if (!isLexicalScope) {
		lexicalScopes.push_back(scopes.back());
	}
	scopes.push_back(scope);
}

Scope* Resolver::peekScope() { return scopes.back(); }

Scope* Resolver::popScope() {
	auto result = peekScope();
	scopes.pop_back();
	if (!lexicalScopes.empty() && result == lexicalScopes.back()) {
		lexicalScopes.pop_back();
	}

	if (isFunctionScope(result)) {
		List<Symbol*> newSymbols;
		for (auto& symbol : result->symbols) {
			if (dynamic_cast<Parameter*>(symbol) ||
				dynamic_cast<GenericType*>(symbol)) {
				newSymbols.push_back(symbol);
			}
		}
		result->symbols.clear();
		for (auto& ns : newSymbols) result->symbols.push_back(ns);
	}

	return result;
}

Scope* Resolver::getLexicalScope() {
	return !lexicalScopes.empty() ? lexicalScopes.back() : peekScope();
}

Resolver::Resolver(CompilerContext& ctx, Ast* ast)
	: ctx(ctx), ast(ast), maxStage(ResolutionStage::RESOLVED) {}

Resolver::Resolver(CompilerContext& ctx, Ast* ast, ResolutionStage maxStage)
	: ctx(ctx), ast(ast), maxStage(maxStage) {}

void Resolver::resolve() {
	ast->stage++;
	do {
		if (ast->stage == ResolutionStage::EXTERNAL_TYPES) {
			ImportHandler ih = ImportHandler(ctx, ast);
			ih.resolveImports();
		}

		pushScope(ast->globalScope, true);
		for (auto& c : ast->globalScope->content) {
			resolveNonLocalContent(c);
		}
		popScope();
		ast->stage++;
	} while (ast->stage < maxStage);
}

void Resolver::resolveNonLocalContent(Node* n) {
	if (Class* c = dynamic_cast<Class*>(n))
		resolveClass(c);
	else if (Struct* c = dynamic_cast<Struct*>(n))
		resolveStruct(c);
	else if (Template* c = dynamic_cast<Template*>(n))
		resolveTemplate(c);
	else if (Enum* c = dynamic_cast<Enum*>(n))
		resolveEnum(c);
	else if (Namespace* c = dynamic_cast<Namespace*>(n))
		resolveNamespace(c);
	else if (Alias* c = dynamic_cast<Alias*>(n))
		resolveAlias(c);
	else if (Variable* c = dynamic_cast<Variable*>(n))
		resolveVariable(c);
	else if (EnumCase* c = dynamic_cast<EnumCase*>(n))
		resolveEnumCase(c);
	else if (Constructor* c = dynamic_cast<Constructor*>(n))
		resolveConstructor(c);
	else if (Function* c = dynamic_cast<Function*>(n))
		resolveFunction(c);
	else
		throw "Unknown node";
}

void Resolver::resolveClass(Class* n) {
	pushScope(n, true);
	for (auto& g : n->generics) resolveGenericType(g);
	for (auto& p : n->parentTypes) resolveTypeRef(p);
	for (auto& c : n->content) resolveNonLocalContent(c);
	popScope();
}

void Resolver::resolveStruct(Struct* n) {
	pushScope(n, true);
	for (auto& g : n->generics) resolveGenericType(g);
	for (auto& p : n->parentTypes) resolveTypeRef(p);
	for (auto& c : n->content) resolveNonLocalContent(c);
	popScope();
}

void Resolver::resolveTemplate(Template* n) {
	pushScope(n, true);
	for (auto& g : n->generics) resolveGenericType(g);
	for (auto& p : n->parentTypes) resolveTypeRef(p);
	for (auto& c : n->content) resolveNonLocalContent(c);
	popScope();
}

void Resolver::resolveEnum(Enum* n) {
	pushScope(n, true);
	for (auto& g : n->generics) resolveGenericType(g);
	for (auto& p : n->parentTypes) resolveTypeRef(p);
	for (auto& c : n->content) resolveNonLocalContent(c);
	popScope();
}

void Resolver::resolveNamespace(Namespace* n) {
	pushScope(n, true);
	for (auto& g : n->generics) resolveGenericType(g);
	for (auto& c : n->content) resolveNonLocalContent(c);
	popScope();
}

void Resolver::resolveAlias(Alias* n) {
	pushScope(n, true);
	for (auto& g : n->generics) resolveGenericType(g);
	resolveTypeRef(n->value);
	popScope();
}

void Resolver::resolveVariable(Variable* n) {
	pushSymbol(n);

	if (!n->actualType && n->declaredType) {
		resolveTypeRef(n->declaredType);
		n->actualType = n->declaredType;
	}

	if (n->value && ast->stage != ResolutionStage::INTERNAL_TYPES &&
		ast->stage != ResolutionStage::EXTERNAL_TYPES) {
		if (VariableBlock* vb = dynamic_cast<VariableBlock*>(n->value)) {
			// TODO: Resolve variable block

			if (!n->actualType) {
				// TODO: Get type based on variable block
			}
		} else {
			Expression* e = dynamic_cast<Expression*>(n->value);
			resolveExpression(e);
			if (!n->actualType) {
				n->actualType = e->valueType;
			}
		}
	}

	popSymbol();
}

void Resolver::resolveEnumCase(EnumCase* n) {
	if (ast->stage != ResolutionStage::INTERNAL_TYPES &&
		ast->stage != ResolutionStage::EXTERNAL_TYPES) {
		for (auto& e : n->args) resolveExpression(e);
		// TODO: Check to make sure the arguments are valid for the enum
	}
}

void Resolver::resolveConstructor(Constructor* n) {
	pushScope(n, true);
	for (auto& p : n->parameters) resolveParameter(p, nullptr);

	if (ast->stage != ResolutionStage::INTERNAL_TYPES &&
		ast->stage != ResolutionStage::EXTERNAL_TYPES) {
		for (auto& c : n->content) resolveLocalContent(c, nullptr);
	}
	popScope();
}

void Resolver::resolveFunction(Function* n) {
	pushSymbol(n);
	pushScope(n, true);
	for (auto& g : n->generics) resolveGenericType(g);
	for (auto& p : n->parameters) resolveParameter(p, nullptr);

	if (!n->actualReturnType) {
		if (n->declaredReturnType) {
			resolveTypeRef(n->declaredReturnType);
			n->actualReturnType = n->declaredReturnType;
		} else if (!n->hasBody)
			n->actualReturnType = tb::base(
				const_cast<bt::InvariantType*>(bt::VOID), {}, n->sourceMeta);
	}

	if (ast->stage != ResolutionStage::INTERNAL_TYPES &&
		ast->stage != ResolutionStage::EXTERNAL_TYPES) {
		try {
			TypeRef* returnType = nullptr;
			for (auto& c : n->content) resolveLocalContent(c, &returnType);
			if (!n->actualReturnType) n->actualReturnType = returnType;
		} catch (RecursiveResolutionException& e) {
		}
	}

	popScope();
	popSymbol();
}

void Resolver::resolveGenericType(GenericType* n) {
	// We don't want to resolve the same thing more than once
	if (n->actualParentType) return;

	if (n->declaredParentType) {
		resolveTypeRef(n->declaredParentType);
		n->actualParentType = n->declaredParentType;
	} else {
		n->actualParentType = tb::base(const_cast<bt::InvariantType*>(bt::ANY),
									   {}, n->sourceMeta);
	}
}

static bool hasGenericType(const List<GenericType*>& list, const String& id) {
	for (const auto& g : list) {
		if (g->id->data == id) return true;
	}
	return false;
}

static TypeRef* generateGenericType(List<GenericType*>& dest,
									const SourceMeta& typeMeta,
									const SourceMeta& refMeta) {
	int suffix = 1;
	String id = "T";
	while (hasGenericType(dest, id)) {
		StringBuffer sb;
		sb << "T" << suffix;
		id = sb.str();
		suffix++;
	}
	auto type =
		new GenericType(new Token(TokenType::ID, id, typeMeta), nullptr);
	dest.push_back(type);

	return tb::base(type, {}, refMeta);
}

void Resolver::resolveParameter(Parameter* n, TypeRef* intendedType) {
	// We don't want to resolve the same thing more than once
	if (n->actualType) return;

	if (n->declaredType) {
		resolveTypeRef(n->declaredType);
		n->actualType = n->declaredType;
	} else if (intendedType) {
		// We have to "copy" the intended type because we don't want the
		// parameter to delete the original intended type when it's type to
		// delete the parameter
		n->actualType =
			tb::base(intendedType->actualType, {}, intendedType->sourceMeta);
		n->actualType->actualGenerics = intendedType->actualGenerics;
	} else if (Function* f = dynamic_cast<Function*>(peekScope())) {
		n->actualType =
			generateGenericType(f->generics, f->sourceMeta, n->sourceMeta);
	} else {
		n->actualType = tb::base(const_cast<bt::InvariantType*>(bt::ANY), {},
								 n->sourceMeta);
	}
}

void Resolver::resolveLocalContent(Node* n, TypeRef** destReturnType) {
	if (Variable* e = dynamic_cast<Variable*>(n))
		resolveVariable(e);
	else if (FunctionBlock* e = dynamic_cast<FunctionBlock*>(n))
		resolveFunctionBlock(e, destReturnType);
	else if (IfBlock* e = dynamic_cast<IfBlock*>(n))
		resolveIfBlock(e, destReturnType);
	else if (WhileBlock* e = dynamic_cast<WhileBlock*>(n))
		resolveWhileBlock(e, destReturnType);
	else if (RepeatBlock* e = dynamic_cast<RepeatBlock*>(n))
		resolveRepeatBlock(e, destReturnType);
	else if (ForBlock* e = dynamic_cast<ForBlock*>(n))
		resolveForBlock(e, destReturnType);
	else if (SwitchBlock* e = dynamic_cast<SwitchBlock*>(n))
		resolveSwitchBlock(e, destReturnType);
	else if (TryBlock* e = dynamic_cast<TryBlock*>(n))
		resolveTryBlock(e, destReturnType);
	else if (Expression* e = dynamic_cast<Expression*>(n))
		resolveExpression(e);
	else if (Alias* e = dynamic_cast<Alias*>(n))
		resolveAlias(e);
	else if (ReturnStatement* e = dynamic_cast<ReturnStatement*>(n))
		resolveReturnStatement(e, destReturnType);
	else if (ThrowStatement* e = dynamic_cast<ThrowStatement*>(n))
		resolveThrowStatement(e);
	else if (dynamic_cast<SingleTokenStatement*>(n))
		return;
	else
		throw AclException(ASP_CORE_UNKNOWN, n->sourceMeta,
						   "Unknown local content");
}

void Resolver::resolveFunctionBlock(FunctionBlock* n,
									TypeRef** destReturnType) {
	pushScope(n, true);
	for (auto& c : n->content) resolveLocalContent(c, destReturnType);
	popScope();
}

void Resolver::resolveIfBlock(IfBlock* n, TypeRef** destReturnType) {
	auto boolRef = tb::base(const_cast<bt::InvariantType*>(bt::BOOL), {},
							bt::BOOL->sourceMeta);

	pushScope(n->block, true);

	resolveExpression(n->condition);
	if (!type::canCastTo(n->condition->valueType, boolRef)) {
		delete boolRef;
		throw AclException(ASP_CORE_UNKNOWN, n->condition->sourceMeta,
						   "Expected Bool type");
	}

	for (auto& c : n->block->content) resolveLocalContent(c, destReturnType);

	popScope();

	for (auto& elif : n->elifBlocks) {
		pushScope(elif->block, true);
		resolveExpression(elif->condition);
		if (!type::canCastTo(elif->condition->valueType, boolRef)) {
			delete boolRef;
			throw AclException(ASP_CORE_UNKNOWN, elif->condition->sourceMeta,
							   "Expected Bool type");
		}
		for (auto& c : elif->block->content)
			resolveLocalContent(c, destReturnType);
		popScope();
	}

	if (n->elseBlock) {
		resolveFunctionBlock(n->elseBlock, destReturnType);
	}

	delete boolRef;
}

void Resolver::resolveWhileBlock(WhileBlock* n, TypeRef** destReturnType) {
	auto boolRef = tb::base(const_cast<bt::InvariantType*>(bt::BOOL), {},
							bt::BOOL->sourceMeta);

	pushScope(n->block, true);

	resolveExpression(n->condition);
	if (!type::canCastTo(n->condition->valueType, boolRef)) {
		delete boolRef;
		throw AclException(ASP_CORE_UNKNOWN, n->condition->sourceMeta,
						   "Expected Bool type");
	}

	for (auto& c : n->block->content) resolveLocalContent(c, destReturnType);

	popScope();

	delete boolRef;
}

void Resolver::resolveRepeatBlock(RepeatBlock* n, TypeRef** destReturnType) {
	auto boolRef = tb::base(const_cast<bt::InvariantType*>(bt::BOOL), {},
							bt::BOOL->sourceMeta);

	pushScope(n->block, true);

	resolveExpression(n->condition);
	if (!type::canCastTo(n->condition->valueType, boolRef)) {
		delete boolRef;
		throw AclException(ASP_CORE_UNKNOWN, n->condition->sourceMeta,
						   "Expected Bool type");
	}

	for (auto& c : n->block->content) resolveLocalContent(c, destReturnType);

	popScope();

	delete boolRef;
}

void Resolver::resolveForBlock(ForBlock* n, TypeRef** destReturnType) {
	pushScope(n->block, true);

	resolveExpression(n->iteratee);

	resolveParameter(n->iterator,
					 getIteratorElementType(n->iteratee->valueType));

	for (auto& c : n->block->content) resolveLocalContent(c, destReturnType);

	popScope();
}

void Resolver::resolveSwitchBlock(SwitchBlock* n, TypeRef** destReturnType) {
	resolveExpression(n->condition);

	for (auto& c : n->cases) {
		if (c->condition) resolveExpression(c->condition);
		resolveFunctionBlock(c->block, destReturnType);
	}
}

void Resolver::resolveTryBlock(TryBlock* n, TypeRef** destReturnType) {
	resolveFunctionBlock(n->block, destReturnType);
	for (auto& c : n->catchBlocks) {
		pushScope(c->block, true);
		resolveParameter(c->exceptionVariable, nullptr);
		for (auto& e : c->block->content)
			resolveLocalContent(e, destReturnType);
		popScope();
	}
}

void Resolver::resolveReturnStatement(ReturnStatement* n,
									  TypeRef** destReturnType) {
	auto f = getOwningFunction(peekScope());

	TypeRef* returnType = nullptr;

	if (n->value) {
		if (!n->value->valueType) {
			try {
				resolveExpression(n->value);
				returnType = n->value->valueType;
			} catch (RecursiveResolutionException& e) {
				if (ast->stage != ResolutionStage::INTERNAL_ALL &&
					ast->stage != ResolutionStage::RESOLVED)
					throw e;
				if (Function* func = dynamic_cast<Function*>(f)) {
					auto g = generateGenericType(
						func->generics, func->sourceMeta, n->sourceMeta);
					returnType = g;
				} else {
					returnType =
						tb::base(const_cast<bt::InvariantType*>(bt::ANY), {},
								 n->sourceMeta);
				}
			}
		} else
			returnType = n->value->valueType;
	} else {
		returnType = tb::base(const_cast<bt::InvariantType*>(bt::VOID), {},
							  n->sourceMeta);
	}

	if (Function* func = dynamic_cast<Function*>(f)) {
		if (func->declaredReturnType &&
			!type::canCastTo(returnType, func->declaredReturnType)) {
			throw AclException(ASP_CORE_UNKNOWN, n->sourceMeta,
							   "Invalid return statement");
		}
	} else if (destReturnType) {
		if (!*destReturnType)
			*destReturnType = returnType;
		else if (((*destReturnType)->actualType == bt::VOID ||
				  returnType->actualType == bt::VOID) &&
				 (*destReturnType)->actualType != returnType->actualType) {
			throw AclException(
				ASP_CORE_UNKNOWN, n->sourceMeta,
				"Cannot return void and non-void values in the same function");
		} else if (returnType->actualType != bt::VOID) {
			*destReturnType = const_cast<TypeRef*>(
				type::getMinCommonType(*destReturnType, returnType));
		}
	}
}

void Resolver::resolveThrowStatement(ThrowStatement* n) {
	resolveExpression(n->value);
	// TODO: Implement this (require value to be subtype or equal to "Exception"
	// type)
}

#pragma region TypeRef

void Resolver::resolveTypeRef(TypeRef* n) {
	// We don't want to resolve the same thing more than once
	if (n->actualType) return;

	if (SimpleTypeRef* c = dynamic_cast<SimpleTypeRef*>(n))
		resolveSimpleTypeRef(c);
	else if (ArrayTypeRef* c = dynamic_cast<ArrayTypeRef*>(n))
		resolveArrayTypeRef(c);
	else if (MapTypeRef* c = dynamic_cast<MapTypeRef*>(n))
		resolveMapTypeRef(c);
	else if (TupleTypeRef* c = dynamic_cast<TupleTypeRef*>(n))
		resolveTupleTypeRef(c);
	else if (FunctionTypeRef* c = dynamic_cast<FunctionTypeRef*>(n))
		resolveFunctionTypeRef(c);
	else if (SuffixTypeRef* c = dynamic_cast<SuffixTypeRef*>(n))
		resolveSuffixTypeRef(c);
	else
		throw "Unimplemented type ref";
}

void Resolver::resolveSimpleTypeRef(SimpleTypeRef* n) {
	for (auto& g : n->generics) {
		resolveTypeRef(g);
		n->actualGenerics.push_back(g);
	}
	if (n->parent) pushScope(resolveSimpleTypeRefParent(n->parent), false);
	auto scope = peekScope();

	SearchCriteria searchCriteria = {!n->parent,
									 ast->stage > ResolutionStage::INTERNAL_ALL,
									 {SearchTarget::TYPE},
									 true,
									 false};
	List<resolve::SearchResult> results;
	resolveSymbol(results, scope, n->id, searchCriteria.recursive,
				  searchCriteria.allowExternal, searchCriteria.targets);
	n->referent = getSymbolReferent(results, n->generics, searchCriteria,
									n->sourceMeta, getLexicalScope());

	n->actualType = dynamic_cast<Type*>(n->referent);

	if (n->parent) popScope();
}

Scope* Resolver::resolveSimpleTypeRefParent(SimpleTypeRef* n) {
	for (auto& g : n->generics) {
		resolveTypeRef(g);
		n->actualGenerics.push_back(g);
	}
	if (n->parent) pushScope(resolveSimpleTypeRefParent(n->parent), false);
	auto scope = peekScope();

	SearchCriteria searchCriteria = {
		!n->parent,
		ast->stage > ResolutionStage::INTERNAL_ALL,
		{SearchTarget::TYPE, SearchTarget::NAMESPACE},
		true,
		false};
	List<resolve::SearchResult> results;
	resolveSymbol(results, scope, n->id, searchCriteria.recursive,
				  searchCriteria.allowExternal, searchCriteria.targets);
	n->referent = getSymbolReferent(results, n->generics, searchCriteria,
									n->sourceMeta, getLexicalScope());

	if (n->parent) popScope();

	return dynamic_cast<Scope*>(n->referent);
}

void Resolver::resolveArrayTypeRef(ArrayTypeRef* n) {
	resolveTypeRef(n->elementType);
	n->actualType = const_cast<bt::InvariantType*>(bt::ARRAY);
	n->actualGenerics.push_back(n->elementType);
}

void Resolver::resolveMapTypeRef(MapTypeRef* n) {
	resolveTypeRef(n->keyType);
	resolveTypeRef(n->valueType);
	n->actualType = const_cast<bt::InvariantType*>(bt::MAP);
	n->actualGenerics.push_back(n->keyType);
	n->actualGenerics.push_back(n->valueType);
}

void Resolver::resolveTupleTypeRef(TupleTypeRef* n) {
	for (auto& e : n->elementTypes) {
		resolveTypeRef(e);
		n->actualGenerics.push_back(e);
	}
	n->actualType = const_cast<bt::InvariantType*>(bt::TUPLE);
}

void Resolver::resolveFunctionTypeRef(FunctionTypeRef* n) {
	resolveTypeRef(n->returnType);
	n->actualGenerics.push_back(n->returnType);
	for (auto& e : n->paramTypes) {
		resolveTypeRef(e);
		n->actualGenerics.push_back(e);
	}
	n->actualType = const_cast<bt::InvariantType*>(bt::FUNCTION);
}

void Resolver::resolveSuffixTypeRef(SuffixTypeRef* n) {
	if (n->suffixSymbol->type == TokenType::QUESTION_MARK) {
		resolveTypeRef(n->type);
		n->actualGenerics.push_back(n->type);
		n->actualType = const_cast<bt::InvariantType*>(bt::OPTIONAL);
	} else if (n->suffixSymbol->type == TokenType::EXCLAMATION_POINT) {
		resolveTypeRef(n->type);
		n->actualGenerics.push_back(n->type);
		n->actualType = const_cast<bt::InvariantType*>(bt::UNWRAPPED_OPTIONAL);
	} else if (n->suffixSymbol->type == TokenType::ASTERISK) {
		resolveTypeRef(n->type);
		n->actualGenerics.push_back(n->type);
		n->actualType = const_cast<bt::InvariantType*>(bt::POINTER);
	} else if (n->suffixSymbol->type == TokenType::TRIPLE_DOT) {
		resolveTypeRef(n->type);
		n->actualGenerics.push_back(n->type);
		n->actualType = const_cast<bt::InvariantType*>(bt::ARRAY);
	} else
		throw "Unknown type ref suffix";
}

#pragma endregion

#pragma region Expression
void Resolver::resolveExpression(Expression* n) {
	resolveExpression0(n, nullptr, nullptr);
}

void Resolver::resolveExpression0(Expression* n,
								  const SearchCriteria* searchCriteria,
								  IdentifierExpression** dest) {
	// Don't resolve the same expression more than once
	if (n->valueType) return;

	if (FunctionCallExpression* c = dynamic_cast<FunctionCallExpression*>(n))
		resolveFunctionCallExpression(c, searchCriteria);
	else if (TernaryExpression* c = dynamic_cast<TernaryExpression*>(n))
		resolveTernaryExpression(c);
	else if (BinaryExpression* c = dynamic_cast<BinaryExpression*>(n))
		resolveBinaryExpression(c, searchCriteria, dest);
	else if (UnaryPrefixExpression* c = dynamic_cast<UnaryPrefixExpression*>(n))
		resolvePrefixExpression(c);
	else if (UnaryPostfixExpression* c =
				 dynamic_cast<UnaryPostfixExpression*>(n))
		resolvePostfixExpression(c);
	else if (SubscriptExpression* c = dynamic_cast<SubscriptExpression*>(n))
		resolveSubscriptExpression(c);
	else if (IdentifierExpression* c = dynamic_cast<IdentifierExpression*>(n))
		resolveIdentifierExpression(c, searchCriteria, dest);
	else if (ArrayLiteralExpression* c =
				 dynamic_cast<ArrayLiteralExpression*>(n))
		resolveArrayLiteralExpression(c);
	else if (MapLiteralExpression* c = dynamic_cast<MapLiteralExpression*>(n))
		resolveMapLiteralExpression(c);
	else if (TupleLiteralExpression* c =
				 dynamic_cast<TupleLiteralExpression*>(n))
		resolveTupleLiteralExpression(c);
	else if (LiteralExpression* c = dynamic_cast<LiteralExpression*>(n))
		resolveLiteralExpression(c);
	else if (LambdaExpression* c = dynamic_cast<LambdaExpression*>(n))
		resolveLambdaExpression(c);
	else if (CastingExpression* c = dynamic_cast<CastingExpression*>(n))
		resolveCastingExpression(c);
}

void Resolver::resolveFunctionCallExpression(
	FunctionCallExpression* n, const SearchCriteria* searchCriteria) {
	SearchCriteria actualCriteria = {
		searchCriteria ? searchCriteria->recursive : true,
		ast->stage > ResolutionStage::INTERNAL_ALL,
		{SearchTarget::VARIABLE, SearchTarget::TYPE},
		false,
		false};
	IdentifierExpression* idexpr = nullptr;
	resolveExpression0(n->caller, &actualCriteria, &idexpr);

	List<TypeRef*> argTypes;
	for (auto& e : n->args) {
		resolveExpression(e);
		argTypes.push_back(e->valueType);
	}

	if (idexpr) {
		// If we're dealing with an identifier expression as the target, we need
		// to find the best fit given the argument types
		TypeRef* returnType = nullptr;
		auto trueCaller = getBestCallerForArgs(idexpr, argTypes, n->sourceMeta,
											   getLexicalScope(),
											   actualCriteria, &returnType);
		idexpr->referent = trueCaller;
		n->valueType = returnType;
	} else {
		// Require the caller expression to be a function expression
		FunctionTypeRef* f =
			dynamic_cast<FunctionTypeRef*>(n->caller->valueType);
		if (!f)
			throw AclException(ASP_CORE_UNKNOWN, n->caller->sourceMeta,
							   "Invalid caller type for function call");

		// Make sure arguments are compatible
		validateFunctionCallArgs(f->paramTypes, argTypes, n->sourceMeta);

		n->valueType = f->returnType;
	}
}

void Resolver::resolveTernaryExpression(TernaryExpression* n) {
	resolveExpression(n->arg0);
	auto boolRef =
		tb::base(const_cast<bt::InvariantType*>(bt::BOOL), {}, n->sourceMeta);
	if (!type::canCastTo(n->arg0->valueType, boolRef)) {
		delete boolRef;
		throw AclException(
			ASP_CORE_UNKNOWN, n->arg0->sourceMeta,
			"Cannot cast condition argument of ternary expression to Bool");
	}

	resolveExpression(n->arg1);
	resolveExpression(n->arg2);

	TypeRef* a = n->arg1->valueType;
	TypeRef* b = n->arg2->valueType;

	delete boolRef;

	n->valueType = const_cast<TypeRef*>(type::getMinCommonType(a, b));
}

void Resolver::resolveBinaryExpression(BinaryExpression* n,
									   const SearchCriteria* searchCriteria,
									   IdentifierExpression** dest) {
	if (n->op->type == TokenType::DOT ||
		n->op->type == TokenType::QUESTION_MARK_DOT) {
		resolveAccessExpression(n, searchCriteria, dest);
	} else {
		resolveExpression(n->left);
		resolveExpression(n->right);
		// TODO: Left arg needs to have the correct function that supports the
		// right arg
	}
}

void Resolver::resolveAccessExpression(BinaryExpression* n,
									   const SearchCriteria* searchCriteria,
									   IdentifierExpression** dest) {
	if (n->op->type == TokenType::DOT) {
		SearchCriteria leftCriteria = {
			true,
			ast->stage > ResolutionStage::INTERNAL_ALL,
			{SearchTarget::NAMESPACE, SearchTarget::VARIABLE,
			 SearchTarget::TYPE},
			true,
			false};
		resolveExpression0(n->left, &leftCriteria, nullptr);
		pushScope(getScopeFromExpression(n->left), false);

		List<SearchTarget> actualTargets;
		if (searchCriteria)
			actualTargets.insert(actualTargets.end(),
								 searchCriteria->targets.begin(),
								 searchCriteria->targets.end());
		else {
			actualTargets.push_back(SearchTarget::NAMESPACE);
			actualTargets.push_back(SearchTarget::VARIABLE);
			actualTargets.push_back(SearchTarget::TYPE);
		}
		SearchCriteria actualCriteria = {
			false, ast->stage > ResolutionStage::INTERNAL_ALL, actualTargets,
			searchCriteria ? searchCriteria->requireExactMatch : true,
			searchCriteria ? searchCriteria->modifiable : false};
		resolveExpression0(n->right, &actualCriteria, dest);

		popScope();

		n->valueType = n->right->valueType;
	} else if (n->op->type == TokenType::QUESTION_MARK_DOT) {
		resolveExpression(n->left);

		SuffixTypeRef* s = dynamic_cast<SuffixTypeRef*>(n->left->valueType);
		if (!s || s->suffixSymbol->type != TokenType::QUESTION_MARK ||
			s->suffixSymbol->type != TokenType::EXCLAMATION_POINT)
			throw AclException(ASP_CORE_UNKNOWN, n->left->sourceMeta,
							   "Expected optional type for first argument to "
							   "optional access expression");

		pushScope(getScopeFromTypeRef(s->type), false);

		List<SearchTarget> actualTargets;
		if (searchCriteria)
			actualTargets.insert(actualTargets.end(),
								 searchCriteria->targets.begin(),
								 searchCriteria->targets.end());
		else {
			actualTargets.push_back(SearchTarget::NAMESPACE);
			actualTargets.push_back(SearchTarget::VARIABLE);
			actualTargets.push_back(SearchTarget::TYPE);
		}
		SearchCriteria actualCriteria = {
			false, ast->stage > ResolutionStage::INTERNAL_ALL, actualTargets,
			searchCriteria ? searchCriteria->requireExactMatch : true,
			searchCriteria ? searchCriteria->modifiable : false};
		resolveExpression0(n->right, &actualCriteria, dest);

		popScope();

		if (s->suffixSymbol->type == TokenType::QUESTION_MARK)
			n->valueType = tb::optional(n->right->valueType);
		else
			n->valueType = tb::unwrappedOptional(n->right->valueType);
	} else {
		throw AclException(ASP_CORE_UNKNOWN, n->sourceMeta,
						   "Invalid access expression operator");
	}
}

void Resolver::resolvePrefixExpression(UnaryPrefixExpression* n) {
	// TODO: Implement this
}

void Resolver::resolvePostfixExpression(UnaryPostfixExpression* n) {
	// TODO: Implement this
}

void Resolver::resolveSubscriptExpression(SubscriptExpression* n) {
	// TODO: Implement this
}

void Resolver::resolveIdentifierExpression(IdentifierExpression* n,
										   const SearchCriteria* searchCriteria,
										   IdentifierExpression** dest) {
	if (dest) *dest = n;
	for (auto& g : n->generics) resolveTypeRef(g);

	List<SearchTarget> actualTargets;
	if (searchCriteria)
		actualTargets.insert(actualTargets.end(),
							 searchCriteria->targets.begin(),
							 searchCriteria->targets.end());
	else {
		actualTargets.push_back(SearchTarget::TYPE);
		actualTargets.push_back(SearchTarget::VARIABLE);
	}

	SearchCriteria actualCriteria = {
		searchCriteria ? searchCriteria->recursive : true,
		ast->stage > ResolutionStage::INTERNAL_ALL, actualTargets,
		searchCriteria ? searchCriteria->requireExactMatch : true,
		searchCriteria ? searchCriteria->modifiable : false};

	List<resolve::SearchResult> results;
	resolveSymbol(results, peekScope(), n->value, actualCriteria.recursive,
				  actualCriteria.allowExternal, actualCriteria.targets);
	if (results.empty())
		throw AclException(ASP_CORE_UNKNOWN, n->sourceMeta,
						   "Unresolved symbol");
	if (actualCriteria.requireExactMatch) {
		n->referent = getSymbolReferent(results, n->generics, actualCriteria,
										n->sourceMeta, getLexicalScope());
		n->valueType = getSymbolReturnType(n->referent, n->sourceMeta);
	} else
		n->possibleReferents.insert(n->possibleReferents.end(), results.begin(),
									results.end());
}

void Resolver::resolveArrayLiteralExpression(ArrayLiteralExpression* n) {
	const TypeRef* t = nullptr;
	for (auto& e : n->elements) {
		resolveExpression(e);
		if (!t)
			t = e->valueType;
		else
			t = type::getMinCommonType(t, e->valueType);
	}
	n->valueType = tb::array(const_cast<TypeRef*>(t));
}

void Resolver::resolveMapLiteralExpression(MapLiteralExpression* n) {
	const TypeRef* k = nullptr;
	const TypeRef* v = nullptr;

	for (auto& key : n->keys) {
		resolveExpression(key);
		if (!k)
			k = key->valueType;
		else
			k = type::getMinCommonType(k, key->valueType);
	}

	for (auto& value : n->values) {
		resolveExpression(value);
		if (!v)
			v = value->valueType;
		else
			v = type::getMinCommonType(v, value->valueType);
	}

	n->valueType = tb::map(const_cast<TypeRef*>(k), const_cast<TypeRef*>(v));
}

void Resolver::resolveTupleLiteralExpression(TupleLiteralExpression* n) {
	List<TypeRef*> elementTypes;

	for (auto& e : n->elements) {
		resolveExpression(e);
		elementTypes.push_back(e->valueType);
	}

	n->valueType = tb::tuple(elementTypes);
}

void Resolver::resolveLiteralExpression(LiteralExpression* n) {
	if (n->value->type == TokenType::FLOAT_LITERAL)
		n->valueType = tb::base(const_cast<bt::InvariantType*>(bt::DOUBLE), {},
								n->sourceMeta);
	else if (n->value->type == TokenType::BOOLEAN_LITERAL)
		n->valueType = tb::base(const_cast<bt::InvariantType*>(bt::BOOL), {},
								n->sourceMeta);
	else if (n->value->type == TokenType::INTEGER_LITERAL ||
			 n->value->type == TokenType::BINARY_LITERAL ||
			 n->value->type == TokenType::OCTAL_LITERAL ||
			 n->value->type == TokenType::HEX_LITERAL)
		n->valueType = tb::base(const_cast<bt::InvariantType*>(bt::INT), {},
								n->sourceMeta);
	else if (n->value->type == TokenType::NIL_LITERAL)
		n->valueType = tb::unwrappedOptional(tb::base(
			const_cast<bt::InvariantType*>(bt::ANY), {}, n->sourceMeta));
	else if (n->value->type == TokenType::STRING_LITERAL)
		n->valueType = tb::base(const_cast<bt::InvariantType*>(bt::STRING), {},
								n->sourceMeta);
	else if (n->value->type == TokenType::SELF) {
		Scope* currentScope = peekScope();
		while (isFunctionScope(currentScope))
			currentScope = currentScope->parentScope;

		Type* t = dynamic_cast<Type*>(currentScope);
		if (!t)
			throw AclException(ASP_CORE_UNKNOWN, n->sourceMeta,
							   "Invalid location for \"self\"");
		if (isStaticContext(peekScope()))
			throw AclException(ASP_CORE_UNKNOWN, n->sourceMeta,
							   "Cannot reference \"self\" in a static context");

		List<TypeRef*> generics;
		for (auto& g : t->generics) {
			generics.push_back(tb::base(g, {}, n->sourceMeta));
		}

		n->valueType = tb::base(t, generics, n->sourceMeta);
	} else if (n->value->type == TokenType::SUPER) {
		Scope* currentScope = peekScope();
		while (isFunctionScope(currentScope))
			currentScope = currentScope->parentScope;

		Type* t = dynamic_cast<Type*>(currentScope);
		if (!t)
			throw AclException(ASP_CORE_UNKNOWN, n->sourceMeta,
							   "Invalid location for \"super\"");
		if (isStaticContext(peekScope()))
			throw AclException(
				ASP_CORE_UNKNOWN, n->sourceMeta,
				"Cannot reference \"super\" in a static context");

		List<TypeRef*> generics;
		for (auto& g : t->generics) {
			generics.push_back(tb::base(g, {}, n->sourceMeta));
		}

		n->valueType = new SuperTypeRef(n->sourceMeta, t);
	} else
		throw AclException(ASP_CORE_UNKNOWN, n->sourceMeta,
						   "Unknown literal expression type");
}

void Resolver::resolveLambdaExpression(LambdaExpression* n) {
	pushScope(n, true);

	List<TypeRef*> paramTypes;
	for (auto& p : n->parameters) {
		resolveParameter(p, nullptr);
		paramTypes.push_back(p->actualType);
	}

	TypeRef* returnType = nullptr;
	for (auto& c : n->content) resolveLocalContent(c, &returnType);

	popScope();

	n->valueType = tb::function(paramTypes, returnType);
}

void Resolver::resolveCastingExpression(CastingExpression* n) {
	resolveExpression(n->left);
	resolveTypeRef(n->right);

	if (n->op->type == TokenType::IS) {
		n->valueType = tb::base(const_cast<bt::InvariantType*>(bt::BOOL), {},
								n->sourceMeta);
	} else if (n->op->type == TokenType::AS) {
		n->valueType = n->right;
	} else if (n->op->type == TokenType::AS_OPTIONAL) {
		n->valueType = tb::optional(n->right);
	} else if (n->op->type == TokenType::AS_UNWRAPPED) {
		n->valueType = tb::unwrappedOptional(n->right);
	} else
		throw AclException(ASP_CORE_UNKNOWN, n->sourceMeta,
						   "Invalid casting operator");
}

#pragma endregion

#pragma region HelperFunctions
TypeRef* Resolver::getSymbolReturnType(Symbol* symbol,
									   const SourceMeta& refererMeta) {
	if (Variable* n = dynamic_cast<Variable*>(symbol)) {
		if (!n->actualType) {
			if (stackContainsSymbol(n))
				throw AclException(
					ASP_CORE_UNKNOWN, refererMeta,
					"Cannot reference a variable before it is defined");
			resolveVariable(n);
		}
		return n->actualType;
	}
	if (Parameter* n = dynamic_cast<Parameter*>(symbol)) return n->actualType;
	if (EnumCase* n = dynamic_cast<EnumCase*>(symbol))
		return tb::base(n->enumType, {}, symbol->sourceMeta);
	if (Function* n = dynamic_cast<Function*>(symbol)) {
		if (!n->actualReturnType) {
			if (stackContainsSymbol(n)) throw RecursiveResolutionException();
			resolveFunction(n);
		}
		List<TypeRef*> paramTypes;
		for (auto& p : n->parameters) paramTypes.push_back(p->actualType);
		return tb::function(paramTypes, n->actualReturnType);
	}
	if (dynamic_cast<Type*>(symbol) || dynamic_cast<Namespace*>(symbol))
		return nullptr;
	throw AclException(ASP_CORE_UNKNOWN, symbol->sourceMeta,
					   "Cannot get return type for symbol");
}

void Resolver::getFunctionCallCandidateType(
	Symbol* symbol, List<std::pair<Symbol*, FunctionTypeRef*>>& refs,
	const SourceMeta& callerMeta) {
	if (dynamic_cast<Variable*>(symbol) || dynamic_cast<Parameter*>(symbol)) {
		refs.push_back(std::make_pair(
			symbol, dynamic_cast<FunctionTypeRef*>(
						getSymbolReturnType(symbol, callerMeta))));
	} else if (Function* f = dynamic_cast<Function*>(symbol)) {
		if (!f->actualReturnType) {
			if (stackContainsSymbol(f)) throw RecursiveResolutionException();
			resolveFunction(f);
		}
		List<TypeRef*> paramTypes;
		for (auto& p : f->parameters) paramTypes.push_back(p->actualType);
		refs.push_back(std::make_pair(
			symbol, tb::function(paramTypes, f->actualReturnType)));
	} else if (EnumCase* e = dynamic_cast<EnumCase*>(symbol)) {
		throw AclException(
			ASP_CORE_UNKNOWN, symbol->sourceMeta,
			"Enum cases cannot be the caller of a function call expression");
	} else if (Constructor* c = dynamic_cast<Constructor*>(symbol)) {
		Type* owningType = dynamic_cast<Type*>(c->parentScope);
		List<TypeRef*> paramTypes;
		for (auto& p : c->parameters) paramTypes.push_back(p->actualType);
		refs.push_back(std::make_pair(
			symbol, tb::function(paramTypes, tb::base(owningType, {},
													  symbol->sourceMeta))));
	} else if (Type* t = dynamic_cast<Type*>(symbol)) {
		return getFcctForType(t, refs, callerMeta);
	} else
		throw AclException(ASP_CORE_UNKNOWN, symbol->sourceMeta,
						   "Unknown symbol");
}

void Resolver::getFunctionCallCandidateTypes(
	const List<resolve::SearchResult>& candidates,
	List<List<std::pair<Symbol*, FunctionTypeRef*>>>& candidateTypes,
	const SourceMeta& callerMeta) {
	for (auto& c : candidates) {
		List<std::pair<Symbol*, FunctionTypeRef*>> refs;
		getFunctionCallCandidateType(c.symbol, refs, callerMeta);
		candidateTypes.push_back(refs);
	}
}

Symbol* Resolver::getBestCallerForArgs(IdentifierExpression* idexpr,
									   const List<TypeRef*>& args,
									   const SourceMeta& callerMeta,
									   Scope* lexicalScope,
									   const SearchCriteria& searchCriteria,
									   TypeRef** destReturnType) {
	List<List<std::pair<Symbol*, FunctionTypeRef*>>> candidateTypes;
	getFunctionCallCandidateTypes(idexpr->possibleReferents, candidateTypes,
								  callerMeta);

	List<List<FCCandidate>> candidateScores;
	getFunctionCallCandidateScores(candidateTypes, args, candidateScores,
								   callerMeta);

	List<std::pair<int, int>> indices;
	sortFccScores(candidateScores, indices);

	Symbol* result = nullptr;
	for (auto& e : indices) {
		if (candidateScores[std::get<0>(e)][std::get<1>(e)].score == -1)
			continue;
		else {
			// TODO: Validate caller here (for things like visibility,
			// staticness, generics, etc.)
			List<SymbolCandidateProblem> problems;
			findSymbolCandidateProblems(
				getFccSearchResult(
					idexpr->possibleReferents[std::get<0>(e)],
					std::get<0>(
						candidateTypes[std::get<0>(e)][std::get<1>(e)])),
				idexpr->generics, searchCriteria, callerMeta, lexicalScope,
				problems);

			if (!problems.empty()) {
				for (const auto& problem : problems) {
					StringBuffer sb;
					sb << "Error " << problem.ec << " in module "
					   << problem.location.file << " at line "
					   << problem.location.line << ", col "
					   << problem.location.col;
					String s = sb.str();
					log::warn(s);
				}

				throw AclException(ASP_CORE_UNKNOWN, callerMeta,
								   "Unresolved symbol");
			}

			result =
				std::get<0>(candidateTypes[std::get<0>(e)][std::get<1>(e)]);
			*destReturnType =
				std::get<1>(candidateTypes[std::get<0>(e)][std::get<1>(e)])
					->returnType;
			break;
		}
	}

	if (!result)
		throw AclException(ASP_CORE_UNKNOWN, callerMeta,
						   "There are no candidate functions or function-like "
						   "objects that accept the provided arguments");

	return result;
}

void Resolver::getFcctForType(Type* type,
							  List<std::pair<Symbol*, FunctionTypeRef*>>& refs,
							  const SourceMeta& callerMeta) {
	if (Class* n = dynamic_cast<Class*>(type)) {
		for (auto& s : n->symbols) {
			if (Constructor* c = dynamic_cast<Constructor*>(s)) {
				List<std::pair<Symbol*, FunctionTypeRef*>> tmprefs;
				getFunctionCallCandidateType(c, tmprefs, callerMeta);
				refs.push_back(tmprefs[0]);
			}
		}
	} else if (Struct* n = dynamic_cast<Struct*>(type)) {
		for (auto& s : n->symbols) {
			if (Constructor* c = dynamic_cast<Constructor*>(s)) {
				List<std::pair<Symbol*, FunctionTypeRef*>> tmprefs;
				getFunctionCallCandidateType(c, tmprefs, callerMeta);
				refs.push_back(tmprefs[0]);
			}
		}
	} else if (Template* n = dynamic_cast<Template*>(type)) {
		throw AclException(ASP_CORE_UNKNOWN, type->sourceMeta,
						   "Templates do not have constructors");
	} else if (Enum* n = dynamic_cast<Enum*>(type)) {
		for (auto& s : n->symbols) {
			if (Constructor* c = dynamic_cast<Constructor*>(s)) {
				List<std::pair<Symbol*, FunctionTypeRef*>> tmprefs;
				getFunctionCallCandidateType(c, tmprefs, callerMeta);
				refs.push_back(tmprefs[0]);
			}
		}
	} else if (Alias* n = dynamic_cast<Alias*>(type)) {
		// TODO: Generics might need to be handled here somehow...
		getFcctForType(n->value->actualType, refs, callerMeta);
	} else
		throw AclException(ASP_CORE_UNKNOWN, type->sourceMeta, "Unknown type");
}
#pragma endregion
}  // namespace acl