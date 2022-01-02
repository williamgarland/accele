#include "ast.hpp"

#include <algorithm>
#include <filesystem>

#include "exceptions.hpp"
#include "invariant_types.hpp"
#include "json_util.hpp"
#include "type_builder.hpp"

namespace acl {
Scope::Scope(Scope* parentScope) : parentScope(parentScope) {}

Scope::~Scope() {}

void Scope::addSymbol(Symbol* symbol) {
	bool isType = dynamic_cast<Type*>(symbol);

	for (const auto& s : symbols) {
		if (((isType && dynamic_cast<Type*>(s)) || !dynamic_cast<Type*>(s)) &&
			s->id->data == symbol->id->data)
			throw AclException(ASP_CORE_UNKNOWN, symbol->id->meta,
							   "Duplicate symbol");
	}

	symbols.push_back(symbol);

	if (isType) types.push_back(dynamic_cast<Type*>(symbol));
}

namespace ResolveFlag {
bool isLexicalOnly(int flags) {
	return ((flags & LEXICAL) == LEXICAL) && ((flags & TYPE_HIERARCHY) == 0);
}
bool isTypeHierarchyOnly(int flags) {
	return ((flags & LEXICAL) == 0) &&
		   ((flags & TYPE_HIERARCHY) == TYPE_HIERARCHY);
}
bool hasRequireExactMatch(int flags) {
	return (flags & REQUIRE_EXACT_MATCH) == REQUIRE_EXACT_MATCH;
}
bool hasRecursive(int flags) { return (flags & RECURSIVE) == RECURSIVE; }

bool flagsAcceptSymbolType(int flags, Symbol* s) {
	return (((flags & TARGET_TYPE) == TARGET_TYPE) && dynamic_cast<Type*>(s)) ||
		   (((flags & TARGET_NAMESPACE) == TARGET_NAMESPACE) &&
			(dynamic_cast<Namespace*>(s) || dynamic_cast<Import*>(s))) ||
		   (((flags & TARGET_VARIABLE) == TARGET_VARIABLE) &&
			(dynamic_cast<Variable*>(s) || dynamic_cast<Function*>(s)));
}
}  // namespace ResolveFlag

namespace type {
static int getTypeMatchScore0(Type** commonTypeDest, Type* a, Type* b,
							  bool traceAll, bool traceB) {
	if (a == b) {
		// This could be set to a or b; it doesn't matter
		if (commonTypeDest) *commonTypeDest = a;
		return 0;
	}

	Type* commonTypeA = nullptr;
	Type* commonTypeB = nullptr;
	int minA = -1;
	int minB = -1;

	for (auto& p : a->parentTypes) {
		Type* commonType = nullptr;
		int s = getTypeMatchScore0(&commonType, getTypeForTypeRef(p), b,
								   traceAll, false);
		if (minA == -1 || s < minA) {
			commonTypeA = commonType;
			minA = s;
		}
	}

	if (traceAll || traceB) {
		for (auto& p : b->parentTypes) {
			Type* commonType = nullptr;
			int s = getTypeMatchScore0(&commonType, getTypeForTypeRef(p), a,
									   traceAll, false);
			if (minB == -1 || s < minB) {
				commonTypeB = commonType;
				minB = s;
			}
		}
	}

	if (minA > -1 && minB > -1) {
		if (minB < minA) {
			if (commonTypeDest) *commonTypeDest = commonTypeB;
			return minB + 1;
		} else {
			if (commonTypeDest) *commonTypeDest = commonTypeA;
			return minA + 1;
		}
	}

	if (minA > -1) {
		if (commonTypeDest) *commonTypeDest = commonTypeA;
		return minA + 1;
	}

	if (minB > -1) {
		if (commonTypeDest) *commonTypeDest = commonTypeB;
		return minB + 1;
	}

	// TODO: Handle the case where at least one of the types is an InvariantType

	return -1;
}

int getTypeMatchScore(Type** commonTypeDest, Type* a, Type* b, bool traceAll) {
	return getTypeMatchScore0(commonTypeDest, a, b, traceAll, true);
}

Type* getTypeForTypeRef(TypeRef* tr) {
	if (SimpleTypeRef* s = dynamic_cast<SimpleTypeRef*>(tr))
		return dynamic_cast<Type*>(s->referent);
	return nullptr;	 // TODO: Implement the rest
}

bool typesMatch(TypeRef* a, TypeRef* b) {
	return getTypeMatchScore(nullptr, getTypeForTypeRef(a),
							 getTypeForTypeRef(b), false) == 0;
}

bool typesAreCompatible(TypeRef* a, TypeRef* b) {
	return getTypeMatchScore(nullptr, getTypeForTypeRef(a),
							 getTypeForTypeRef(b), false) >= 0;
}

void getGenerics(List<GenericType*>& dest, Symbol* s) {
	if (Function* f = dynamic_cast<Function*>(s)) {
		dest.insert(dest.end(), f->generics.begin(), f->generics.end());
	} else if (Type* t = dynamic_cast<Type*>(s)) {
		dest.insert(dest.end(), t->generics.begin(), t->generics.end());
	} else if (Namespace* n = dynamic_cast<Namespace*>(s)) {
		dest.insert(dest.end(), n->generics.begin(), n->generics.end());
	}
}

bool genericsAreCompatible(const List<TypeRef*>& supplied,
						   const List<GenericType*>& target) {
	if (supplied.size() > target.size()) return false;
	for (int i = 0; i < supplied.size(); i++)
		if (!genericAcceptsType(target[i], supplied[i])) return false;
	return true;
}

bool genericAcceptsType(GenericType* g, TypeRef* t) {
	auto gt = getTypeForGeneric(g);
	auto tt = getTypeForTypeRef(t);
	return getTypeMatchScore(nullptr, gt, tt, false) >= 0;
}

Type* getTypeForGeneric(GenericType* g) {
	if (g->declaredParentType) return getTypeForTypeRef(g->declaredParentType);
	return const_cast<bt::InvariantType*>(bt::ANY);
}

Type* getMinCommonType(Type* a, Type* b) {
	Type* result = nullptr;
	int score = getTypeMatchScore(&result, a, b, true);
	if (score == -1)
		// This error should never occur; all types have at least a min common
		// type of "Any"
		throw AclException(ASP_CORE_UNKNOWN, a->sourceMeta,
						   "Incompatible types");
	return result;
}

bool canCastTo(Type* src, Type* target) {
	Type* dest = nullptr;
	int score = getTypeMatchScore(&dest, src, target, false);
	return score >= 0 && dest == target;
}
}  // namespace type

static bool isObjectOrientedScope(Scope* scope) {
	return dynamic_cast<Class*>(scope) || dynamic_cast<Struct*>(scope) ||
		   dynamic_cast<Template*>(scope) || dynamic_cast<Enum*>(scope);
}

static bool hasStaticModifier(const List<Modifier*>& modifiers) {
	for (const auto& m : modifiers) {
		if (m->content->type == TokenType::STATIC) return true;
	}
	return false;
}

static bool isSymbolStatic(Symbol* s, bool checkModifierOnly = true) {
	if (Variable* v = dynamic_cast<Variable*>(s))
		return hasStaticModifier(v->modifiers);
	if (Function* f = dynamic_cast<Function*>(s))
		return hasStaticModifier(f->modifiers);
	return !checkModifierOnly &&
		   (dynamic_cast<Type*>(s) || dynamic_cast<Namespace*>(s) ||
			dynamic_cast<Import*>(s));
}

static TypeRef* getTypeForSymbol(Symbol* s) {
	if (Function* f = dynamic_cast<Function*>(s)) {
		List<TypeRef*> paramTypes;
		for (auto& p : f->parameters) {
			if (p->declaredType)
				paramTypes.push_back(p->declaredType);
			else
				paramTypes.push_back(
					tb::base(const_cast<bt::InvariantType*>(bt::ANY), {},
							 p->sourceMeta));
		}
		return tb::function(paramTypes, f->actualReturnType);
	}

	return dynamic_cast<Variable*>(s)->actualType;
}

void Scope::resolveSymbol(List<Symbol*>& dest, const Token* id,
						  TypeRef* expectedType, const List<TypeRef*>& generics,
						  int flags) {
	for (auto& s : symbols) {
		if ((isObjectOrientedScope(this) && !isSymbolStatic(s) &&
			 ResolveFlag::isLexicalOnly(flags)) ||
			(isSymbolStatic(s, false) &&
			 ResolveFlag::isTypeHierarchyOnly(flags)))
			continue;

		List<GenericType*> genericTypes;
		type::getGenerics(genericTypes, s);

		if (ResolveFlag::flagsAcceptSymbolType(flags, s) &&
			s->id->data == id->data &&
			type::genericsAreCompatible(generics, genericTypes) &&
			(!expectedType ||
			 (ResolveFlag::hasRequireExactMatch(flags) &&
			  type::typesMatch(getTypeForSymbol(s), expectedType)) ||
			 (!ResolveFlag::hasRequireExactMatch(flags) &&
			  type::typesAreCompatible(getTypeForSymbol(s), expectedType)))) {
			dest.push_back(s);
		}
	}

	if (ResolveFlag::hasRecursive(flags)) {
		if (!ResolveFlag::isTypeHierarchyOnly(flags)) {
			if (parentScope) {
				try {
					List<Symbol*> foundParentSymbols;
					int parentFlags = flags;
					parentFlags |= ResolveFlag::LEXICAL;
					parentFlags &= ~ResolveFlag::TYPE_HIERARCHY;
					parentScope->resolveSymbol(foundParentSymbols, id,
											   expectedType, generics,
											   parentFlags);

					for (auto& s : foundParentSymbols) dest.push_back(s);
				} catch (AclException& e) {
				}
			} else if (GlobalScope* gs = dynamic_cast<GlobalScope*>(this)) {
				for (auto& imp : gs->imports) {
					try {
						List<Symbol*> foundImportSymbols;
						int importFlags = flags;
						importFlags &= ~ResolveFlag::RECURSIVE;
						importFlags |= ResolveFlag::LEXICAL;
						importFlags &= ~ResolveFlag::TYPE_HIERARCHY;
						imp->referent->resolveSymbol(foundImportSymbols, id,
													 expectedType, generics,
													 importFlags);

						for (auto& s : foundImportSymbols) dest.push_back(s);
					} catch (AclException& e) {
					}
				}
			}
		}
	}

	if (!ResolveFlag::isLexicalOnly(flags)) {
		if (Type* t = dynamic_cast<Type*>(this)) {
			for (auto& parent : t->parentTypes) {
				try {
					auto type = type::getTypeForTypeRef(parent);
					if (Scope* asScope = dynamic_cast<Scope*>(type)) {
						List<Symbol*> foundParentSymbols;
						int parentFlags = flags;
						parentFlags |= ResolveFlag::LEXICAL;
						parentFlags &= ~ResolveFlag::TYPE_HIERARCHY;
						asScope->resolveSymbol(foundParentSymbols, id,
											   expectedType, generics,
											   parentFlags);

						for (auto& s : foundParentSymbols) dest.push_back(s);
					}
				} catch (AclException& e) {
				}
			}
		}
	}

	if (ResolveFlag::hasRequireExactMatch(flags) && dest.size() > 1)
		throw AclException(ASP_CORE_UNKNOWN, id->meta,
						   "Multiple symbols match the specified criteria");
	if (dest.size() < 1)
		throw AclException(ASP_CORE_UNKNOWN, id->meta, "Unresolved symbol");
}

bool hasCompatibleGenerics(const Type* type, const List<TypeRef*>& generics) {
	// TODO: Implement this
	return true;
}

Node::Node(const SourceMeta& sourceMeta) : sourceMeta(sourceMeta) {}

Node::~Node() {}

Ast::Ast(GlobalScope* globalScope)
	: globalScope(globalScope), stage(ResolutionStage::INITIAL_PASS) {}

Ast::~Ast() { delete globalScope; }

GlobalScope::GlobalScope(const SourceMeta& sourceMeta,
						 const List<Node*>& content)
	: Symbol(new Token(TokenType::GLOBAL, "global", sourceMeta)),
	  Scope(nullptr),
	  content(content) {}

GlobalScope::~GlobalScope() {
	for (auto& c : content) delete c;
}

void GlobalScope::toJson(StringBuffer& dest) const {
	dest << "{\n\"name\":\"GlobalScope\",\n\"content\": ";
	json::appendList<Node*>(dest, content,
							[](auto& d, const auto& e) { e->toJson(d); });
	dest << "\n}";
}

void GlobalScope::addImport(Import* imp) {
	if (!imp->alias)
		throw AclException(ASP_CORE_UNKNOWN, imp->sourceMeta,
						   "Import has no alias");
	for (const auto& other : imports) {
		if (other->alias->data == imp->alias->data)
			throw AclException(ASP_CORE_UNKNOWN, imp->sourceMeta,
							   "Duplicate import");
	}
	imports.push_back(imp);
}

Import* GlobalScope::resolveImport(Token* id) {
	for (auto& imp : imports)
		if (imp->alias->data == id->data) return imp;
	throw AclException(ASP_CORE_UNKNOWN, id->meta, "Unresolved import");
}

TypeRef::TypeRef(const SourceMeta& sourceMeta) : Node(sourceMeta) {}

TypeRef::~TypeRef() {}

SimpleTypeRef::SimpleTypeRef(const SourceMeta& sourceMeta, Token* id,
							 const List<TypeRef*>& generics,
							 SimpleTypeRef* parent)
	: TypeRef(sourceMeta), id(id), generics(generics), parent(parent) {}

SimpleTypeRef::~SimpleTypeRef() {
	delete id;
	for (auto& c : generics) delete c;
	delete parent;
}

void SimpleTypeRef::toJson(StringBuffer& dest) const {
	dest << "{\n\"name\":\"SimpleTypeRef\",\n";
	dest << "\"id\": \"" << id->data << "\",\n";
	dest << "\"generics\": ";
	json::appendList<TypeRef*>(dest, generics,
							   [](auto& d, const auto& e) { e->toJson(d); });
	dest << ",\n\"parent\": ";
	if (parent)
		parent->toJson(dest);
	else
		dest << "null";
	dest << "\n}";
}

SuffixTypeRef::SuffixTypeRef(const SourceMeta& sourceMeta, TypeRef* type,
							 Token* suffixSymbol)
	: TypeRef(sourceMeta), type(type), suffixSymbol(suffixSymbol) {}

SuffixTypeRef::~SuffixTypeRef() {
	delete type;
	delete suffixSymbol;
}

void SuffixTypeRef::toJson(StringBuffer& dest) const {
	dest << "{\n\"name\":\"SuffixTypeRef\",\n";
	dest << "\"type\": ";
	type->toJson(dest);
	dest << ",\n\"suffixSymbol\": \"" << suffixSymbol->data << "\"\n}";
}

TupleTypeRef::TupleTypeRef(const SourceMeta& sourceMeta,
						   const List<TypeRef*>& elementTypes)
	: TypeRef(sourceMeta),
	  elementTypes(elementTypes),
	  deleteElementTypes(true) {}

TupleTypeRef::~TupleTypeRef() {
	if (deleteElementTypes)
		for (auto& c : elementTypes) delete c;
}

void TupleTypeRef::toJson(StringBuffer& dest) const {
	dest << "{\n\"name\":\"TupleTypeRef\",\n";
	dest << "\"elementTypes\": ";
	json::appendList<TypeRef*>(dest, elementTypes,
							   [](auto& d, const auto& e) { e->toJson(d); });
	dest << "\n}";
}

MapTypeRef::MapTypeRef(const SourceMeta& sourceMeta, TypeRef* keyType,
					   TypeRef* valueType)
	: TypeRef(sourceMeta), keyType(keyType), valueType(valueType) {}

MapTypeRef::~MapTypeRef() {
	delete keyType;
	delete valueType;
}

void MapTypeRef::toJson(StringBuffer& dest) const {
	dest << "{\n\"name\":\"MapTypeRef\",\n";
	dest << "\"keyType\": ";
	keyType->toJson(dest);
	dest << ",\n\"valueType\": ";
	valueType->toJson(dest);
	dest << "\n}";
}

ArrayTypeRef::ArrayTypeRef(const SourceMeta& sourceMeta, TypeRef* elementType)
	: TypeRef(sourceMeta), elementType(elementType) {}

ArrayTypeRef::~ArrayTypeRef() { delete elementType; }

void ArrayTypeRef::toJson(StringBuffer& dest) const {
	dest << "{\n\"name\":\"ArrayTypeRef\",\n";
	dest << "\"elementType\": ";
	elementType->toJson(dest);
	dest << "\n}";
}

FunctionTypeRef::FunctionTypeRef(const SourceMeta& sourceMeta,
								 const List<TypeRef*>& paramTypes,
								 TypeRef* returnType)
	: TypeRef(sourceMeta), paramTypes(paramTypes), returnType(returnType) {}

FunctionTypeRef::~FunctionTypeRef() {
	for (auto& c : paramTypes) delete c;
	delete returnType;
}

void FunctionTypeRef::toJson(StringBuffer& dest) const {
	dest << "{\n\"name\":\"FunctionTypeRef\",\n";
	dest << "\"paramTypes\": ";
	json::appendList<TypeRef*>(dest, paramTypes,
							   [](auto& d, const auto& e) { e->toJson(d); });
	dest << ",\n\"returnType\": ";
	returnType->toJson(dest);
	dest << "\n}";
}

Expression::Expression(const SourceMeta& sourceMeta) : Node(sourceMeta) {}

Expression::~Expression() {}

TernaryExpression::TernaryExpression(const SourceMeta& sourceMeta,
									 Expression* arg0, Expression* arg1,
									 Expression* arg2)
	: Expression(sourceMeta), arg0(arg0), arg1(arg1), arg2(arg2) {}

TernaryExpression::~TernaryExpression() {
	delete arg0;
	delete arg1;
	delete arg2;
}

void TernaryExpression::toJson(StringBuffer& dest) const {
	dest << "{\n\"name\": \"TernaryExpression\",\n";
	dest << "\"arg0\": ";
	arg0->toJson(dest);
	dest << ",\n\"arg1\": ";
	arg1->toJson(dest);
	dest << ",\n\"arg2\": ";
	arg2->toJson(dest);
	dest << "\n}";
}

BinaryExpression::BinaryExpression(const SourceMeta& sourceMeta, Token* op,
								   Expression* left, Expression* right)
	: Expression(sourceMeta), op(op), left(left), right(right) {}

BinaryExpression::~BinaryExpression() {
	delete op;
	delete left;
	delete right;
}

void BinaryExpression::toJson(StringBuffer& dest) const {
	dest << "{\n\"name\": \"BinaryExpression\",\n";
	dest << "\"op\": \"" << op->data << "\",\n";
	dest << "\"left\": ";
	left->toJson(dest);
	dest << ",\n\"right\": ";
	right->toJson(dest);
	dest << "\n}";
}

UnaryPrefixExpression::UnaryPrefixExpression(const SourceMeta& sourceMeta,
											 Token* op, Expression* arg)
	: Expression(sourceMeta), op(op), arg(arg) {}

UnaryPrefixExpression::~UnaryPrefixExpression() {
	delete op;
	delete arg;
}

void UnaryPrefixExpression::toJson(StringBuffer& dest) const {
	dest << "{\n\"name\": \"UnaryPrefixExpression\",\n";
	dest << "\"op\": \"" << op->data << "\",\n";
	dest << "\"arg\": ";
	arg->toJson(dest);
	dest << "\n}";
}

UnaryPostfixExpression::UnaryPostfixExpression(const SourceMeta& sourceMeta,
											   Token* op, Expression* arg)
	: Expression(sourceMeta), op(op), arg(arg) {}

UnaryPostfixExpression::~UnaryPostfixExpression() {
	delete op;
	delete arg;
}

void UnaryPostfixExpression::toJson(StringBuffer& dest) const {
	dest << "{\n\"name\": \"UnaryPostfixExpression\",\n";
	dest << "\"op\": \"" << op->data << "\",\n";
	dest << "\"arg\": ";
	arg->toJson(dest);
	dest << "\n}";
}

FunctionCallExpression::FunctionCallExpression(const SourceMeta& sourceMeta,
											   Expression* caller,
											   List<Expression*> args)
	: Expression(sourceMeta), caller(caller), args(args) {}

FunctionCallExpression::~FunctionCallExpression() {
	delete caller;
	for (auto& c : args) delete c;
}

void FunctionCallExpression::toJson(StringBuffer& dest) const {
	dest << "{\n\"name\": \"FunctionCallExpression\",\n";
	dest << "\"caller\": ";
	caller->toJson(dest);
	dest << ",\n\"args\": ";
	json::appendList<Expression*>(dest, args,
								  [](auto& d, const auto& e) { e->toJson(d); });
	dest << "\n}";
}

SubscriptExpression::SubscriptExpression(const SourceMeta& sourceMeta,
										 Expression* target, Expression* index)
	: Expression(sourceMeta), target(target), index(index) {}

SubscriptExpression::~SubscriptExpression() {
	delete target;
	delete index;
}

void SubscriptExpression::toJson(StringBuffer& dest) const {
	dest << "{\n\"name\": \"SubscriptExpression\",\n";
	dest << "\"target\": ";
	target->toJson(dest);
	dest << ",\n\"index\": ";
	index->toJson(dest);
	dest << "\n}";
}

CastingExpression::CastingExpression(const SourceMeta& sourceMeta, Token* op,
									 Expression* left, TypeRef* right)
	: Expression(sourceMeta), op(op), left(left), right(right) {}

CastingExpression::~CastingExpression() {
	delete op;
	delete left;
	delete right;
}

void CastingExpression::toJson(StringBuffer& dest) const {
	dest << "{\n\"name\": \"CastingExpression\",\n";
	dest << "\"op\": \"" << op->data << "\",\n";
	dest << "\"left\": ";
	left->toJson(dest);
	dest << ",\n\"right\": ";
	right->toJson(dest);
	dest << "\n}";
}

MapLiteralExpression::MapLiteralExpression(const SourceMeta& sourceMeta,
										   const List<Expression*>& keys,
										   const List<Expression*>& values)
	: Expression(sourceMeta), keys(keys), values(values) {}

MapLiteralExpression::~MapLiteralExpression() {
	for (auto& c : keys) delete c;
	for (auto& c : values) delete c;
}

void MapLiteralExpression::toJson(StringBuffer& dest) const {
	dest << "{\n\"name\": \"MapLiteralExpression\",\n";
	dest << "\"keys\": ";
	json::appendList<Expression*>(dest, keys,
								  [](auto& d, const auto& e) { e->toJson(d); });
	dest << ",\n\"values\": ";
	json::appendList<Expression*>(dest, values,
								  [](auto& d, const auto& e) { e->toJson(d); });
	dest << "\n}";
}

ArrayLiteralExpression::ArrayLiteralExpression(
	const SourceMeta& sourceMeta, const List<Expression*>& elements)
	: Expression(sourceMeta), elements(elements) {}

ArrayLiteralExpression::~ArrayLiteralExpression() {
	for (auto& c : elements) delete c;
}

void ArrayLiteralExpression::toJson(StringBuffer& dest) const {
	dest << "{\n\"name\": \"ArrayLiteralExpression\",\n";
	dest << "\"elements\": ";
	json::appendList<Expression*>(dest, elements,
								  [](auto& d, const auto& e) { e->toJson(d); });
	dest << "\n}";
}

TupleLiteralExpression::TupleLiteralExpression(
	const SourceMeta& sourceMeta, const List<Expression*>& elements)
	: Expression(sourceMeta), elements(elements) {}

TupleLiteralExpression::~TupleLiteralExpression() {
	for (auto& c : elements) delete c;
}

void TupleLiteralExpression::toJson(StringBuffer& dest) const {
	dest << "{\n\"name\": \"TupleLiteralExpression\",\n";
	dest << "\"elements\": ";
	json::appendList<Expression*>(dest, elements,
								  [](auto& d, const auto& e) { e->toJson(d); });
	dest << "\n}";
}

LiteralExpression::LiteralExpression(Token* value)
	: Expression(value->meta), value(value) {}

LiteralExpression::~LiteralExpression() { delete value; }

void LiteralExpression::toJson(StringBuffer& dest) const {
	dest << "{\n\"name\": \"LiteralExpression\",\n";
	dest << "\"type\": " << (int)value->type << ",\n";
	dest << "\"value\": ";
	json::appendToken(dest, value);
	dest << "\n}";
}

IdentifierExpression::IdentifierExpression(Token* value,
										   const List<TypeRef*>& generics,
										   bool globalPrefix)
	: Expression(value->meta),
	  value(value),
	  generics(generics),
	  globalPrefix(globalPrefix) {}

IdentifierExpression::~IdentifierExpression() {
	delete value;
	for (auto& c : generics) delete c;
}

void IdentifierExpression::toJson(StringBuffer& dest) const {
	dest << "{\n\"name\": \"IdentifierExpression\",\n";
	dest << "\"value\": \"" << value->data << "\",\n";
	dest << "\"generics\": ";
	json::appendList<TypeRef*>(dest, generics,
							   [](auto& d, const auto& e) { e->toJson(d); });
	dest << ",\n\"globalPrefix\": ";
	json::appendBool(dest, globalPrefix);
	dest << "\n}";
}

LambdaExpression::LambdaExpression(const SourceMeta& sourceMeta,
								   const List<Modifier*>& modifiers,
								   const List<Parameter*>& parameters,
								   const List<Node*>& content,
								   Scope* parentScope)
	: Expression(sourceMeta),
	  Scope(parentScope),
	  modifiers(modifiers),
	  parameters(parameters),
	  content(content) {}

LambdaExpression::~LambdaExpression() {
	for (auto& c : modifiers) delete c;
	for (auto& c : parameters) delete c;
	for (auto& c : content) delete c;
}

void LambdaExpression::toJson(StringBuffer& dest) const {
	dest << "{\n\"name\": \"LambdaExpression\",\n";
	dest << "\"modifiers\": ";
	json::appendList<Modifier*>(dest, modifiers,
								[](auto& d, const auto& e) { e->toJson(d); });
	dest << ",\n\"parameters\": ";
	json::appendList<Parameter*>(dest, parameters,
								 [](auto& d, const auto& e) { e->toJson(d); });
	dest << ",\n\"content\": ";
	json::appendList<Node*>(dest, content,
							[](auto& d, const auto& e) { e->toJson(d); });
	dest << "\n}";
}

Symbol::Symbol(Token* id) : Node(id->meta), id(id) {}

Symbol::~Symbol() { delete id; }

Parameter::Parameter(const List<Modifier*>& modifiers, Token* id,
					 TypeRef* declaredType)
	: Symbol(id), modifiers(modifiers), declaredType(declaredType) {}

Parameter::~Parameter() {
	for (auto& c : modifiers) delete c;
	delete declaredType;
}

void Parameter::toJson(StringBuffer& dest) const {
	dest << "{\n\"name\": \"Parameter\",\n";
	dest << "\"modifiers\": ";
	json::appendList<Modifier*>(dest, modifiers,
								[](auto& d, const auto& e) { e->toJson(d); });
	dest << ",\n\"id\": \"" << id->data << "\",\n";
	dest << "\"declaredType\": ";
	if (declaredType)
		declaredType->toJson(dest);
	else
		dest << "null";
	dest << "\n}";
}

FunctionBlock::FunctionBlock(const SourceMeta& sourceMeta,
							 const List<Modifier*>& modifiers,
							 const List<Node*>& content, Scope* parentScope)
	: Node(sourceMeta),
	  Scope(parentScope),
	  modifiers(modifiers),
	  content(content) {}

FunctionBlock::~FunctionBlock() {
	for (auto& c : modifiers) delete c;
	for (auto& c : content) delete c;
}

void FunctionBlock::toJson(StringBuffer& dest) const {
	dest << "{\n\"name\": \"FunctionBlock\",\n";
	dest << "\"modifiers\": ";
	json::appendList<Modifier*>(dest, modifiers,
								[](auto& d, const auto& e) { e->toJson(d); });
	dest << ",\n\"content\": ";
	json::appendList<Node*>(dest, content,
							[](auto& d, const auto& e) { e->toJson(d); });
	dest << "\n}";
}

Function::Function(const List<Modifier*>& modifiers, Token* id,
				   const List<GenericType*>& generics,
				   const List<Parameter*>& parameters,
				   TypeRef* declaredReturnType, const List<Node*>& content,
				   Scope* parentScope)
	: Symbol(id),
	  Scope(parentScope),
	  modifiers(modifiers),
	  generics(generics),
	  parameters(parameters),
	  declaredReturnType(declaredReturnType),
	  actualReturnType(nullptr),
	  content(content) {}

Function::~Function() {
	for (auto& c : modifiers) delete c;
	for (auto& c : generics) delete c;
	for (auto& c : parameters) delete c;
	delete declaredReturnType;
	if (declaredReturnType != actualReturnType) delete actualReturnType;
	for (auto& c : content) delete c;
}

void Function::toJson(StringBuffer& dest) const {
	dest << "{\n\"name\": \"Function\",\n";
	dest << "\"modifiers\": ";
	json::appendList<Modifier*>(dest, modifiers,
								[](auto& d, const auto& e) { e->toJson(d); });
	dest << ",\n\"id\": \"" << id->data << "\",\n";
	dest << "\"generics\": ";
	json::appendList<GenericType*>(
		dest, generics, [](auto& d, const auto& e) { e->toJson(d); });
	dest << ",\n\"parameters\": ";
	json::appendList<Parameter*>(dest, parameters,
								 [](auto& d, const auto& e) { e->toJson(d); });
	dest << ",\n\"declaredReturnType\": ";
	if (declaredReturnType)
		declaredReturnType->toJson(dest);
	else
		dest << "null";
	dest << ",\n\"content\": ";
	json::appendList<Node*>(dest, content,
							[](auto& d, const auto& e) { e->toJson(d); });
	dest << "\n}";
}

Variable::Variable(const List<Modifier*>& modifiers, Token* id,
				   TypeRef* declaredType, Node* value, bool constant)
	: Symbol(id),
	  modifiers(modifiers),
	  declaredType(declaredType),
	  actualType(nullptr),
	  value(value),
	  constant(constant) {}

Variable::~Variable() {
	for (auto& c : modifiers) delete c;
	delete declaredType;
	if (declaredType != actualType) delete actualType;
	delete value;
}

void Variable::toJson(StringBuffer& dest) const {
	dest << "{\n\"name\": \"Variable\",\n";
	dest << "\"modifiers\": ";
	json::appendList<Modifier*>(dest, modifiers,
								[](auto& d, const auto& e) { e->toJson(d); });
	dest << ",\n\"id\": \"" << id->data << "\",\n";
	dest << "\"declaredType\": ";
	if (declaredType)
		declaredType->toJson(dest);
	else
		dest << "null";
	dest << ",\n\"value\": ";
	if (value)
		value->toJson(dest);
	else
		dest << "null";
	dest << ",\n\"constant\": ";
	json::appendBool(dest, constant);
	dest << "\n}";
}

ConditionalBlock::ConditionalBlock(const SourceMeta& sourceMeta,
								   Expression* condition, FunctionBlock* block)
	: Node(sourceMeta), condition(condition), block(block) {}

ConditionalBlock::~ConditionalBlock() {
	delete condition;
	delete block;
}

void ConditionalBlock::toJson(StringBuffer& dest) const {
	dest << "{\n\"name\": \"ConditionalBlock\",\n";
	dest << "\"condition\": ";
	condition->toJson(dest);
	dest << ",\n\"block\": ";
	block->toJson(dest);
	dest << "\n}";
}

IfBlock::IfBlock(const SourceMeta& sourceMeta, Expression* condition,
				 FunctionBlock* block,
				 const List<ConditionalBlock*>& elifBlocks,
				 FunctionBlock* elseBlock)
	: ConditionalBlock(sourceMeta, condition, block),
	  elifBlocks(elifBlocks),
	  elseBlock(elseBlock) {}

IfBlock::~IfBlock() {
	for (auto& c : elifBlocks) delete c;
	delete elseBlock;
}

void IfBlock::toJson(StringBuffer& dest) const {
	dest << "{\n\"name\": \"IfBlock\",\n";
	dest << "\"condition\": ";
	condition->toJson(dest);
	dest << ",\n\"block\": ";
	block->toJson(dest);
	dest << ",\n\"elifBlocks\": ";
	json::appendList<ConditionalBlock*>(
		dest, elifBlocks, [](auto& d, const auto& e) { e->toJson(d); });
	dest << ",\n\"elseBlock\": ";
	if (elseBlock)
		elseBlock->toJson(dest);
	else
		dest << "null";
	dest << "\n}";
}

WhileBlock::WhileBlock(const SourceMeta& sourceMeta, Expression* condition,
					   FunctionBlock* block)
	: ConditionalBlock(sourceMeta, condition, block) {}

WhileBlock::~WhileBlock() {}

void WhileBlock::toJson(StringBuffer& dest) const {
	dest << "{\n\"name\": \"WhileBlock\",\n";
	dest << "\"condition\": ";
	condition->toJson(dest);
	dest << ",\n\"block\": ";
	block->toJson(dest);
	dest << "\n}";
}

RepeatBlock::RepeatBlock(const SourceMeta& sourceMeta, Expression* condition,
						 FunctionBlock* block)
	: ConditionalBlock(sourceMeta, condition, block) {}

RepeatBlock::~RepeatBlock() {}

void RepeatBlock::toJson(StringBuffer& dest) const {
	dest << "{\n\"name\": \"RepeatBlock\",\n";
	dest << "\"condition\": ";
	condition->toJson(dest);
	dest << ",\n\"block\": ";
	block->toJson(dest);
	dest << "\n}";
}

ForBlock::ForBlock(const SourceMeta& sourceMeta, Parameter* iterator,
				   Expression* iteratee, FunctionBlock* block)
	: Node(sourceMeta), iterator(iterator), iteratee(iteratee), block(block) {}

ForBlock::~ForBlock() {
	delete iterator;
	delete iteratee;
	delete block;
}

void ForBlock::toJson(StringBuffer& dest) const {
	dest << "{\n\"name\": \"ForBlock\",\n";
	dest << "\"iterator\": ";
	iterator->toJson(dest);
	dest << ",\n\"iteratee\": ";
	iteratee->toJson(dest);
	dest << ",\n\"block\": ";
	block->toJson(dest);
	dest << "\n}";
}

CatchBlock::CatchBlock(const SourceMeta& sourceMeta,
					   Parameter* exceptionVariable, FunctionBlock* block)
	: Node(sourceMeta), exceptionVariable(exceptionVariable), block(block) {}

CatchBlock::~CatchBlock() {
	delete exceptionVariable;
	delete block;
}

void CatchBlock::toJson(StringBuffer& dest) const {
	dest << "{\n\"name\": \"CatchBlock\",\n";
	dest << "\"exceptionVariable\": ";
	exceptionVariable->toJson(dest);
	dest << ",\n\"block\": ";
	block->toJson(dest);
	dest << "\n}";
}

TryBlock::TryBlock(const SourceMeta& sourceMeta, FunctionBlock* block,
				   const List<CatchBlock*>& catchBlocks)
	: Node(sourceMeta), block(block), catchBlocks(catchBlocks) {}

TryBlock::~TryBlock() {
	delete block;
	for (auto& c : catchBlocks) delete c;
}

void TryBlock::toJson(StringBuffer& dest) const {
	dest << "{\n\"name\": \"TryBlock\",\n";
	dest << "\"block\": ";
	block->toJson(dest);
	dest << ",\n\"catchBlocks\": ";
	json::appendList<CatchBlock*>(dest, catchBlocks,
								  [](auto& d, const auto& e) { e->toJson(d); });
	dest << "\n}";
}

SwitchCaseBlock::SwitchCaseBlock(const SourceMeta& sourceMeta, Token* caseType,
								 Expression* condition, FunctionBlock* block)
	: Node(sourceMeta),
	  caseType(caseType),
	  condition(condition),
	  block(block) {}

SwitchCaseBlock::~SwitchCaseBlock() {
	delete caseType;
	delete condition;
	delete block;
}

void SwitchCaseBlock::toJson(StringBuffer& dest) const {
	dest << "{\n\"name\": \"SwitchCaseBlock\",\n";
	dest << "\"caseType\": \"" << caseType->data << "\",\n";
	dest << "\"condition\": ";
	if (condition)
		condition->toJson(dest);
	else
		dest << "null";
	dest << ",\n\"block\": ";
	block->toJson(dest);
	dest << "\n}";
}

SwitchBlock::SwitchBlock(const SourceMeta& sourceMeta, Expression* condition,
						 const List<SwitchCaseBlock*>& cases)
	: Node(sourceMeta), condition(condition), cases(cases) {}

SwitchBlock::~SwitchBlock() {
	delete condition;
	for (auto& c : cases) delete c;
}

void SwitchBlock::toJson(StringBuffer& dest) const {
	dest << "{\n\"name\": \"SwitchBlock\",\n";
	dest << "\"condition\": ";
	condition->toJson(dest);
	dest << ",\n\"cases\": ";
	json::appendList<SwitchCaseBlock*>(
		dest, cases, [](auto& d, const auto& e) { e->toJson(d); });
	dest << "\n}";
}

ReturnStatement::ReturnStatement(const SourceMeta& sourceMeta,
								 Expression* value)
	: Node(sourceMeta), value(value) {}

ReturnStatement::~ReturnStatement() { delete value; }

void ReturnStatement::toJson(StringBuffer& dest) const {
	dest << "{\n\"name\": \"ReturnStatement\",\n";
	dest << "\"value\": ";
	if (value)
		value->toJson(dest);
	else
		dest << "null";
	dest << "\n}";
}

ThrowStatement::ThrowStatement(const SourceMeta& sourceMeta, Expression* value)
	: Node(sourceMeta), value(value) {}

ThrowStatement::~ThrowStatement() { delete value; }

void ThrowStatement::toJson(StringBuffer& dest) const {
	dest << "{\n\"name\": \"ThrowStatement\",\n";
	dest << "\"value\": ";
	value->toJson(dest);
	dest << "\n}";
}

SingleTokenStatement::SingleTokenStatement(Token* content)
	: Node(content->meta), content(content) {}

SingleTokenStatement::~SingleTokenStatement() { delete content; }

void SingleTokenStatement::toJson(StringBuffer& dest) const {
	dest << "{\n\"name\": \"SingleTokenStatement\",\n";
	dest << "\"content\": \"" << content->data << "\"";
	dest << "\n}";
}

Type::Type(Token* id, const List<GenericType*>& generics)
	: Symbol(id), generics(generics) {}

Type::~Type() {
	for (auto& c : generics) delete c;
}

GenericType::GenericType(Token* id, TypeRef* declaredParentType)
	: Type(id, {}), declaredParentType(declaredParentType) {}

GenericType::~GenericType() { delete declaredParentType; }

void GenericType::toJson(StringBuffer& dest) const {
	dest << "{\n\"name\": \"GenericType\",\n";
	dest << "\"id\": \"" << id->data << "\",\n";
	dest << "\"declaredParentType\": ";
	if (declaredParentType)
		declaredParentType->toJson(dest);
	else
		dest << "null";
	dest << "\n}";
}

Alias::Alias(const List<Modifier*>& modifiers, Token* id,
			 const List<GenericType*>& generics, TypeRef* value)
	: Type(id, generics), modifiers(modifiers), value(value) {}

Alias::~Alias() {
	for (auto& c : modifiers) delete c;
	delete value;
}

void Alias::toJson(StringBuffer& dest) const {
	dest << "{\n\"name\": \"Alias\",\n";
	dest << "\"modifiers\": ";
	json::appendList<Modifier*>(dest, modifiers,
								[](auto& d, const auto& e) { e->toJson(d); });
	dest << ",\n\"id\": \"" << id->data << "\",\n";
	dest << "\"generics\": ";
	json::appendList<GenericType*>(
		dest, generics, [](auto& d, const auto& e) { e->toJson(d); });
	dest << ",\n\"value\": ";
	value->toJson(dest);
	dest << "\n}";
}

SetBlock::SetBlock(const SourceMeta& sourceMeta,
				   const List<Modifier*>& modifiers, Parameter* parameter,
				   const List<Node*>& content, Scope* parentScope)
	: Node(sourceMeta),
	  Scope(parentScope),
	  modifiers(modifiers),
	  parameter(parameter),
	  content(content) {}

SetBlock::~SetBlock() {
	for (auto& c : modifiers) delete c;
	delete parameter;
	for (auto& c : content) delete c;
}

void SetBlock::toJson(StringBuffer& dest) const {
	dest << "{\n\"name\": \"SetBlock\",\n";
	dest << "\"modifiers\": ";
	json::appendList<Modifier*>(dest, modifiers,
								[](auto& d, const auto& e) { e->toJson(d); });
	dest << ",\n\"parameter\": ";
	if (parameter)
		parameter->toJson(dest);
	else
		dest << "null";
	dest << ",\n\"content\": ";
	json::appendList<Node*>(dest, content,
							[](auto& d, const auto& e) { e->toJson(d); });
	dest << "\n}";
}

VariableBlock::VariableBlock(const SourceMeta& sourceMeta,
							 FunctionBlock* getBlock, SetBlock* setBlock,
							 FunctionBlock* initBlock)
	: Node(sourceMeta),
	  getBlock(getBlock),
	  setBlock(setBlock),
	  initBlock(initBlock) {}

VariableBlock::~VariableBlock() {
	delete getBlock;
	delete setBlock;
	delete initBlock;
}

void VariableBlock::toJson(StringBuffer& dest) const {
	dest << "{\n\"name\": \"VariableBlock\",\n";
	dest << "\"getBlock\": ";
	if (getBlock)
		getBlock->toJson(dest);
	else
		dest << "null";
	dest << ",\n\"setBlock\": ";
	if (setBlock)
		setBlock->toJson(dest);
	else
		dest << "null";
	dest << ",\n\"initBlock\": ";
	if (initBlock)
		initBlock->toJson(dest);
	else
		dest << "null";
	dest << "\n}";
}

Class::Class(const List<Modifier*>& modifiers, Token* id,
			 const List<GenericType*>& generics, TypeRef* declaredParentType,
			 const List<TypeRef*>& usedTemplates, const List<Node*>& content,
			 Scope* parentScope)
	: Type(id, generics),
	  Scope(parentScope),
	  modifiers(modifiers),
	  declaredParentType(declaredParentType),
	  usedTemplates(usedTemplates),
	  content(content) {}

Class::~Class() {
	for (auto& c : modifiers) delete c;
	delete declaredParentType;
	for (auto& c : usedTemplates) delete c;
	for (auto& c : content) delete c;
}

void Class::toJson(StringBuffer& dest) const {
	dest << "{\n\"name\": \"Class\",\n";
	dest << "\"modifiers\": ";
	json::appendList<Modifier*>(dest, modifiers,
								[](auto& d, const auto& e) { e->toJson(d); });
	dest << ",\n\"id\": \"" << id->data << "\",\n";
	dest << "\"generics\": ";
	json::appendList<GenericType*>(
		dest, generics, [](auto& d, const auto& e) { e->toJson(d); });
	dest << ",\n\"declaredParentType\": ";
	if (declaredParentType)
		declaredParentType->toJson(dest);
	else
		dest << "null";
	dest << ",\n\"usedTemplates\": ";
	json::appendList<TypeRef*>(dest, usedTemplates,
							   [](auto& d, const auto& e) { e->toJson(d); });
	dest << ",\n\"content\": ";
	json::appendList<Node*>(dest, content,
							[](auto& d, const auto& e) { e->toJson(d); });
	dest << "\n}";
}

Struct::Struct(const List<Modifier*>& modifiers, Token* id,
			   const List<GenericType*>& generics, TypeRef* declaredParentType,
			   const List<TypeRef*>& usedTemplates, const List<Node*>& content,
			   Scope* parentScope)
	: Type(id, generics),
	  Scope(parentScope),
	  modifiers(modifiers),
	  declaredParentType(declaredParentType),
	  usedTemplates(usedTemplates),
	  content(content) {}

Struct::~Struct() {
	for (auto& c : modifiers) delete c;
	delete declaredParentType;
	for (auto& c : usedTemplates) delete c;
	for (auto& c : content) delete c;
}

void Struct::toJson(StringBuffer& dest) const {
	dest << "{\n\"name\": \"Struct\",\n";
	dest << "\"modifiers\": ";
	json::appendList<Modifier*>(dest, modifiers,
								[](auto& d, const auto& e) { e->toJson(d); });
	dest << ",\n\"id\": \"" << id->data << "\",\n";
	dest << "\"generics\": ";
	json::appendList<GenericType*>(
		dest, generics, [](auto& d, const auto& e) { e->toJson(d); });
	dest << ",\n\"declaredParentType\": ";
	if (declaredParentType)
		declaredParentType->toJson(dest);
	else
		dest << "null";
	dest << ",\n\"usedTemplates\": ";
	json::appendList<TypeRef*>(dest, usedTemplates,
							   [](auto& d, const auto& e) { e->toJson(d); });
	dest << ",\n\"content\": ";
	json::appendList<Node*>(dest, content,
							[](auto& d, const auto& e) { e->toJson(d); });
	dest << "\n}";
}

Template::Template(const List<Modifier*>& modifiers, Token* id,
				   const List<GenericType*>& generics,
				   const List<TypeRef*>& declaredParentTypes,
				   const List<Node*>& content, Scope* parentScope)
	: Type(id, generics),
	  Scope(parentScope),
	  modifiers(modifiers),
	  declaredParentTypes(declaredParentTypes),
	  content(content) {}

Template::~Template() {
	for (auto& c : modifiers) delete c;
	for (auto& c : declaredParentTypes) delete c;
	for (auto& c : content) delete c;
}

void Template::toJson(StringBuffer& dest) const {
	dest << "{\n\"name\": \"Template\",\n";
	dest << "\"modifiers\": ";
	json::appendList<Modifier*>(dest, modifiers,
								[](auto& d, const auto& e) { e->toJson(d); });
	dest << ",\n\"id\": \"" << id->data << "\",\n";
	dest << "\"generics\": ";
	json::appendList<GenericType*>(
		dest, generics, [](auto& d, const auto& e) { e->toJson(d); });
	dest << ",\n\"declaredParentTypes\": ";
	json::appendList<TypeRef*>(dest, declaredParentTypes,
							   [](auto& d, const auto& e) { e->toJson(d); });
	dest << ",\n\"content\": ";
	json::appendList<Node*>(dest, content,
							[](auto& d, const auto& e) { e->toJson(d); });
	dest << "\n}";
}

Enum::Enum(const List<Modifier*>& modifiers, Token* id,
		   const List<GenericType*>& generics,
		   const List<TypeRef*>& usedTemplates, const List<Node*>& content,
		   Scope* parentScope)
	: Type(id, generics),
	  Scope(parentScope),
	  modifiers(modifiers),
	  usedTemplates(usedTemplates),
	  content(content) {}

Enum::~Enum() {
	for (auto& c : modifiers) delete c;
	for (auto& c : usedTemplates) delete c;
	for (auto& c : content) delete c;
}

void Enum::toJson(StringBuffer& dest) const {
	dest << "{\n\"name\": \"Enum\",\n";
	dest << "\"modifiers\": ";
	json::appendList<Modifier*>(dest, modifiers,
								[](auto& d, const auto& e) { e->toJson(d); });
	dest << ",\n\"id\": \"" << id->data << "\",\n";
	dest << "\"generics\": ";
	json::appendList<GenericType*>(
		dest, generics, [](auto& d, const auto& e) { e->toJson(d); });
	dest << ",\n\"usedTemplates\": ";
	json::appendList<TypeRef*>(dest, usedTemplates,
							   [](auto& d, const auto& e) { e->toJson(d); });
	dest << ",\n\"content\": ";
	json::appendList<Node*>(dest, content,
							[](auto& d, const auto& e) { e->toJson(d); });
	dest << "\n}";
}

Namespace::Namespace(const List<Modifier*>& modifiers, Token* id,
					 const List<GenericType*>& generics,
					 const List<Node*>& content, Scope* parentScope)
	: Symbol(id),
	  Scope(parentScope),
	  modifiers(modifiers),
	  generics(generics),
	  content(content) {}

Namespace::~Namespace() {
	for (auto& c : modifiers) delete c;
	for (auto& c : generics) delete c;
	for (auto& c : content) delete c;
}

void Namespace::toJson(StringBuffer& dest) const {
	dest << "{\n\"name\": \"Namespace\",\n";
	dest << "\"modifiers\": ";
	json::appendList<Modifier*>(dest, modifiers,
								[](auto& d, const auto& e) { e->toJson(d); });
	dest << ",\n\"id\": \"" << id->data << "\",\n";
	dest << "\"generics\": ";
	json::appendList<GenericType*>(
		dest, generics, [](auto& d, const auto& e) { e->toJson(d); });
	dest << ",\n\"content\": ";
	json::appendList<Node*>(dest, content,
							[](auto& d, const auto& e) { e->toJson(d); });
	dest << "\n}";
}

Constructor::Constructor(const List<Modifier*>& modifiers, Token* id,
						 const List<Parameter*>& parameters,
						 const List<Node*>& content, Scope* parentScope)
	: Symbol(id),
	  Scope(parentScope),
	  modifiers(modifiers),
	  parameters(parameters),
	  content(content) {}

Constructor::~Constructor() {
	for (auto& c : modifiers) delete c;
	for (auto& c : parameters) delete c;
	for (auto& c : content) delete c;
}

void Constructor::toJson(StringBuffer& dest) const {
	dest << "{\n\"name\": \"Constructor\",\n";
	dest << "\"modifiers\": ";
	json::appendList<Modifier*>(dest, modifiers,
								[](auto& d, const auto& e) { e->toJson(d); });
	dest << ",\n\"id\": \"" << id->data << "\",\n";
	dest << "\"parameters\": ";
	json::appendList<Parameter*>(dest, parameters,
								 [](auto& d, const auto& e) { e->toJson(d); });
	dest << ",\n\"content\": ";
	json::appendList<Node*>(dest, content,
							[](auto& d, const auto& e) { e->toJson(d); });
	dest << "\n}";
}

Destructor::Destructor(const SourceMeta& sourceMeta, List<Modifier*>& modifiers,
					   const List<Node*>& content, Scope* parentScope)
	: Node(sourceMeta),
	  Scope(parentScope),
	  modifiers(modifiers),
	  content(content) {}

Destructor::~Destructor() {
	for (auto& c : modifiers) delete c;
	for (auto& c : content) delete c;
}

void Destructor::toJson(StringBuffer& dest) const {
	dest << "{\n\"name\": \"Destructor\",\n";
	dest << "\"modifiers\": ";
	json::appendList<Modifier*>(dest, modifiers,
								[](auto& d, const auto& e) { e->toJson(d); });
	dest << ",\n\"content\": ";
	json::appendList<Node*>(dest, content,
							[](auto& d, const auto& e) { e->toJson(d); });
	dest << "\n}";
}

EnumCase::EnumCase(const List<Modifier*>& modifiers, Token* id,
				   const List<Expression*>& args)
	: Symbol(id), modifiers(modifiers), args(args) {}

EnumCase::~EnumCase() {
	for (auto& c : modifiers) delete c;
	for (auto& c : args) delete c;
}

void EnumCase::toJson(StringBuffer& dest) const {
	dest << "{\n\"name\": \"EnumCase\",\n";
	dest << "\"modifiers\": ";
	json::appendList<Modifier*>(dest, modifiers,
								[](auto& d, const auto& e) { e->toJson(d); });
	dest << ",\n\"id\": \"" << id->data << "\",\n";
	dest << "\"args\": ";
	json::appendList<Expression*>(dest, args,
								  [](auto& d, const auto& e) { e->toJson(d); });
	dest << "\n}";
}

ImportTarget::ImportTarget(Token* id, TypeRef* declaredType)
	: Node(id->meta), id(id), declaredType(declaredType) {}

ImportTarget::~ImportTarget() {
	delete id;
	delete declaredType;
}

void ImportTarget::toJson(StringBuffer& dest) const {
	dest << "{\n\"name\": \"ImportTarget\",\n";
	dest << "\"id\": \"" << id->data << "\",\n";
	dest << "\"declaredType\": ";
	if (declaredType)
		declaredType->toJson(dest);
	else
		dest << "null";
	dest << "\n}";
}

ImportSource::ImportSource(Token* content, ImportSource* parent)
	: Node(content->meta), content(content), parent(parent) {}

ImportSource::~ImportSource() {
	delete content;
	delete parent;
}

void ImportSource::toJson(StringBuffer& dest) const {
	dest << "{\n\"name\": \"ImportSource\",\n";
	dest << "\"content\": \"" << content->data << "\",\n";
	dest << "\"parent\": ";
	if (parent)
		parent->toJson(dest);
	else
		dest << "null";
	dest << "\n}";
}

static String formatImportAlias(const String& str) {
	StringBuffer sb;
	if (!isIdentifierStart(str[0]))
		sb << '_';
	else
		sb << str[0];
	for (int i = 1; i < str.length(); i++) {
		if (!isIdentifierPart(str[i])) sb << '_';
		sb << str[i];
	}
	String result = sb.str();
	return result;
}

static Token* getImportAliasFromSource(ImportSource* src) {
	if (src->content->type == TokenType::ID) return src->content;
	const std::filesystem::path p = src->content->data;
	return new Token(TokenType::ID,
					 formatImportAlias(p.filename().stem().string()),
					 src->content->meta);
}

Import::Import(ImportSource* source, Token* alias,
			   const List<ImportTarget*>& targets)
	: Symbol(source->content), source(source), alias(alias), targets(targets) {
	if (!alias) this->alias = getImportAliasFromSource(source);
}

Import::~Import() {
	delete source;
	delete alias;
	for (auto& c : targets) delete c;
}

void Import::toJson(StringBuffer& dest) const {
	dest << "{\n\"name\": \"Import\",\n";
	dest << "\"source\": ";
	source->toJson(dest);
	dest << ",\n\"alias\": ";
	if (alias)
		dest << "\"" << alias->data << "\"";
	else
		dest << "null";
	dest << ",\n\"targets\": ";
	json::appendList<ImportTarget*>(
		dest, targets, [](auto& d, const auto& e) { e->toJson(d); });
	dest << "\n}";
}

Modifier::Modifier(Token* content) : Node(content->meta), content(content) {}

Modifier::~Modifier() { delete content; }

void Modifier::toJson(StringBuffer& dest) const {
	dest << "{\n\"name\": \"Modifier\",\n";
	dest << "\"content\": \"" << content->data << "\"";
	dest << "\n}";
}

MetaDeclaration::MetaDeclaration(Token* content) : Modifier(content) {}

MetaDeclaration::~MetaDeclaration() {}

void MetaDeclaration::toJson(StringBuffer& dest) const {
	dest << "{\n\"name\": \"MetaDeclaration\",\n";
	dest << "\"content\": \"" << content->data << "\"";
	dest << "\n}";
}

WarningMetaDeclaration::WarningMetaDeclaration(Token* content,
											   const List<Token*>& args,
											   Node* target)
	: MetaDeclaration(content), args(args), target(target) {}

WarningMetaDeclaration::~WarningMetaDeclaration() {
	for (auto& c : args) delete c;
	delete target;
}

void WarningMetaDeclaration::toJson(StringBuffer& dest) const {
	dest << "{\n\"name\": \"WarningMetaDeclaration\",\n";
	dest << "\"content\": \"" << content->data << "\",\n";
	dest << "\"args\": ";
	json::appendList<Token*>(
		dest, args, [](auto& d, const auto& e) { json::appendToken(d, e); });
	dest << ",\n\"target\": ";
	if (target)
		target->toJson(dest);
	else
		dest << "null";
	dest << "\n}";
}

bool isFunctionScope(const Scope* scope) {
	return dynamic_cast<const Function*>(scope) ||
		   dynamic_cast<const FunctionBlock*>(scope) ||
		   dynamic_cast<const LambdaExpression*>(scope) ||
		   dynamic_cast<const SetBlock*>(scope) ||
		   dynamic_cast<const Constructor*>(scope) ||
		   dynamic_cast<const Destructor*>(scope);
}

}  // namespace acl