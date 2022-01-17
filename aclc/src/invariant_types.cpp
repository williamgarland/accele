#include "invariant_types.hpp"

#include "diagnoser.hpp"
#include "type_builder.hpp"

namespace acl {
namespace bt {
InvariantType::InvariantType(const String& id,
							 std::initializer_list<TypeRef*> parentTypes)
	: Type(new Token(TokenType::ID, id, {nullptr, -1, -1}), {}),
	  Scope(nullptr) {
	this->parentTypes.insert(this->parentTypes.end(), parentTypes.begin(),
							 parentTypes.end());
}

InvariantType::InvariantType(const String& id,
							 std::initializer_list<GenericType*> generics)
	: Type(new Token(TokenType::ID, id, {nullptr, -1, -1}), generics),
	  Scope(nullptr) {}

InvariantType::~InvariantType() {
	for (auto& t : parentTypes) {
		delete t;
	}
}

void InvariantType::toJson(StringBuffer& dest) const {
	dest << "{\n\"name\": \"InvariantType\",\n\"id\": \"" << this->id->data
		 << "\"\n}";
}

static InvariantType T_ANY =
	InvariantType("Any", std::initializer_list<TypeRef*>{});
static InvariantType T_NUMBER = InvariantType(
	"Number",
	{tb::base(const_cast<InvariantType*>(&T_ANY), {}, T_ANY.sourceMeta)});
static InvariantType T_INT = InvariantType(
	"Int",
	{tb::base(const_cast<InvariantType*>(&T_NUMBER), {}, T_NUMBER.sourceMeta)});
static InvariantType T_INT8 = InvariantType(
	"Int8",
	{tb::base(const_cast<InvariantType*>(&T_NUMBER), {}, T_NUMBER.sourceMeta)});
static InvariantType T_INT16 = InvariantType(
	"Int16",
	{tb::base(const_cast<InvariantType*>(&T_NUMBER), {}, T_NUMBER.sourceMeta)});
static InvariantType T_INT32 = InvariantType(
	"Int32",
	{tb::base(const_cast<InvariantType*>(&T_NUMBER), {}, T_NUMBER.sourceMeta)});
static InvariantType T_INT64 = InvariantType(
	"Int64",
	{tb::base(const_cast<InvariantType*>(&T_NUMBER), {}, T_NUMBER.sourceMeta)});
static InvariantType T_UINT = InvariantType(
	"UInt",
	{tb::base(const_cast<InvariantType*>(&T_NUMBER), {}, T_NUMBER.sourceMeta)});
static InvariantType T_UINT8 = InvariantType(
	"UInt8",
	{tb::base(const_cast<InvariantType*>(&T_NUMBER), {}, T_NUMBER.sourceMeta)});
static InvariantType T_UINT16 = InvariantType(
	"UInt16",
	{tb::base(const_cast<InvariantType*>(&T_NUMBER), {}, T_NUMBER.sourceMeta)});
static InvariantType T_UINT32 = InvariantType(
	"UInt32",
	{tb::base(const_cast<InvariantType*>(&T_NUMBER), {}, T_NUMBER.sourceMeta)});
static InvariantType T_UINT64 = InvariantType(
	"UInt64",
	{tb::base(const_cast<InvariantType*>(&T_NUMBER), {}, T_NUMBER.sourceMeta)});
static InvariantType T_FLOAT = InvariantType(
	"Float",
	{tb::base(const_cast<InvariantType*>(&T_NUMBER), {}, T_NUMBER.sourceMeta)});
static InvariantType T_DOUBLE = InvariantType(
	"Double",
	{tb::base(const_cast<InvariantType*>(&T_NUMBER), {}, T_NUMBER.sourceMeta)});
static InvariantType T_FLOAT80 = InvariantType(
	"Float80",
	{tb::base(const_cast<InvariantType*>(&T_NUMBER), {}, T_NUMBER.sourceMeta)});
static InvariantType T_BOOL = InvariantType(
	"Bool",
	{tb::base(const_cast<InvariantType*>(&T_ANY), {}, T_ANY.sourceMeta)});
static InvariantType T_STRING = InvariantType(
	"String",
	{tb::base(const_cast<InvariantType*>(&T_ANY), {}, T_ANY.sourceMeta)});
static InvariantType T_VOID = InvariantType(
	"Void",
	std::initializer_list<TypeRef*>{});	 // TODO: Not sure whether this needs to
										 // have a parent type of "Any"...
static InvariantType T_ARRAY = InvariantType(
	"Array",
	{tb::base(const_cast<InvariantType*>(&T_ANY), {}, T_ANY.sourceMeta)});
static InvariantType T_MAP = InvariantType(
	"Map",
	{tb::base(const_cast<InvariantType*>(&T_ANY), {}, T_ANY.sourceMeta)});
static InvariantType T_TUPLE = InvariantType(
	"Tuple",
	{tb::base(const_cast<InvariantType*>(&T_ANY), {}, T_ANY.sourceMeta)});
static InvariantType T_FUNCTION = InvariantType(
	"Function",
	{tb::base(const_cast<InvariantType*>(&T_ANY), {}, T_ANY.sourceMeta)});
static InvariantType T_OPTIONAL = InvariantType(
	"Optional",
	{tb::base(const_cast<InvariantType*>(&T_ANY), {}, T_ANY.sourceMeta)});
static InvariantType T_UNWRAPPED_OPTIONAL = InvariantType(
	"UnwrappedOptional",
	{tb::base(const_cast<InvariantType*>(&T_ANY), {}, T_ANY.sourceMeta)});
static InvariantType T_POINTER = InvariantType(
	"Pointer",
	{tb::base(const_cast<InvariantType*>(&T_ANY), {}, T_ANY.sourceMeta)});
static InvariantType T_ITERATOR = InvariantType(
	"Iterator",
	{new GenericType(new Token(TokenType::ID, "T", {nullptr, -1, -1}),
					 nullptr)});
static InvariantType T_RANGE = InvariantType(
	"Range", {new GenericType(new Token(TokenType::ID, "T", {nullptr, -1, -1}),
							  nullptr)});
static InvariantType T_ITERABLE = InvariantType(
	"Iterable",
	{new GenericType(new Token(TokenType::ID, "T", {nullptr, -1, -1}),
					 nullptr)});

const InvariantType* ANY = &T_ANY;
const InvariantType* NUMBER = &T_NUMBER;
const InvariantType* INT = &T_INT;
const InvariantType* INT8 = &T_INT8;
const InvariantType* INT16 = &T_INT16;
const InvariantType* INT32 = &T_INT32;
const InvariantType* INT64 = &T_INT64;
const InvariantType* UINT = &T_UINT;
const InvariantType* UINT8 = &T_UINT8;
const InvariantType* UINT16 = &T_UINT16;
const InvariantType* UINT32 = &T_UINT32;
const InvariantType* UINT64 = &T_UINT64;
const InvariantType* FLOAT = &T_FLOAT;
const InvariantType* DOUBLE = &T_DOUBLE;
const InvariantType* FLOAT80 = &T_FLOAT80;
const InvariantType* BOOL = &T_BOOL;
const InvariantType* STRING = &T_STRING;
const InvariantType* VOID = &T_VOID;
const InvariantType* ARRAY = &T_ARRAY;
const InvariantType* MAP = &T_MAP;
const InvariantType* TUPLE = &T_TUPLE;
const InvariantType* FUNCTION = &T_FUNCTION;
const InvariantType* OPTIONAL = &T_OPTIONAL;
const InvariantType* UNWRAPPED_OPTIONAL = &T_UNWRAPPED_OPTIONAL;
const InvariantType* POINTER = &T_POINTER;
const InvariantType* ITERATOR = &T_ITERATOR;
const InvariantType* ITERABLE = &T_ITERABLE;
const InvariantType* RANGE = &T_RANGE;

static constexpr int T_INVARIANTS_LEN = 28;

static const InvariantType* T_INVARIANTS[T_INVARIANTS_LEN] = {
	ANY,	 NUMBER,   INT,		INT8,	  INT16,	INT32,
	INT64,	 UINT,	   UINT8,	UINT16,	  UINT32,	UINT64,
	FLOAT,	 DOUBLE,   FLOAT80, BOOL,	  STRING,	VOID,
	ARRAY,	 MAP,	   TUPLE,	FUNCTION, OPTIONAL, UNWRAPPED_OPTIONAL,
	POINTER, ITERATOR, RANGE,	ITERABLE};

bool isInvariantType(const String& id) {
	for (int i = 0; i < T_INVARIANTS_LEN; i++)
		if (T_INVARIANTS[i]->id->data == id) return true;
	return false;
}

const InvariantType* resolveInvariantType(const Token* id) {
	for (int i = 0; i < T_INVARIANTS_LEN; i++)
		if (T_INVARIANTS[i]->id->data == id->data) return T_INVARIANTS[i];
	throw UnresolvedSymbolException(id);
}

static Symbol* func(const String& id, TokenType type,
					std::initializer_list<TypeRef*> params, TypeRef* ret) {
	List<Parameter*> paramSymbols;
	for (auto& p : params) {
		StringBuffer sb;
		sb << "p" << paramSymbols.size();
		String id = sb.str();
		paramSymbols.push_back(new Parameter(
			{}, new Token(TokenType::ID, id, {nullptr, -1, -1}), p));
	}
	return new Function({}, new Token(type, id, {nullptr, -1, -1}), {},
						paramSymbols, ret, {}, nullptr, false);
}

static Symbol* ifunc(const String& id, std::initializer_list<TypeRef*> params,
					 TypeRef* ret) {
	auto idType = getSymbolType(id);
	if (idType == TokenType::EOF_TOKEN) idType = getIdentifierType(id);
	return func(id, idType, params, ret);
}

static Symbol* preop(const String& id, TypeRef* ret) {
	auto idType = getSymbolType(id);
	if (idType == TokenType::EOF_TOKEN) idType = getIdentifierType(id);
	return new Function({new Modifier(new Token(TokenType::PREFIX, "prefix",
												{nullptr, -1, -1}))},
						new Token(idType, id, {nullptr, -1, -1}), {}, {}, ret,
						{}, nullptr, false);
}

static Symbol* postop(const String& id, TypeRef* ret) {
	auto idType = getSymbolType(id);
	if (idType == TokenType::EOF_TOKEN) idType = getIdentifierType(id);
	return new Function({new Modifier(new Token(TokenType::POSTFIX, "postfix",
												{nullptr, -1, -1}))},
						new Token(idType, id, {nullptr, -1, -1}), {}, {}, ret,
						{}, nullptr, false);
}

#define INT_OP_FUNCS(T)                                                       \
	ifunc("&",                                                                \
		  {tb::base(const_cast<InvariantType*>(T), {}, {nullptr, -1, -1})},   \
		  tb::base(const_cast<InvariantType*>(T), {}, {nullptr, -1, -1})),    \
		ifunc(                                                                \
			"|",                                                              \
			{tb::base(const_cast<InvariantType*>(T), {}, {nullptr, -1, -1})}, \
			tb::base(const_cast<InvariantType*>(T), {}, {nullptr, -1, -1})),  \
		ifunc(                                                                \
			"^",                                                              \
			{tb::base(const_cast<InvariantType*>(T), {}, {nullptr, -1, -1})}, \
			tb::base(const_cast<InvariantType*>(T), {}, {nullptr, -1, -1})),  \
		ifunc(                                                                \
			"<<",                                                             \
			{tb::base(const_cast<InvariantType*>(T), {}, {nullptr, -1, -1})}, \
			tb::base(const_cast<InvariantType*>(T), {}, {nullptr, -1, -1})),  \
		ifunc(                                                                \
			">>",                                                             \
			{tb::base(const_cast<InvariantType*>(T), {}, {nullptr, -1, -1})}, \
			tb::base(const_cast<InvariantType*>(T), {}, {nullptr, -1, -1})),  \
		preop("~",                                                            \
			  tb::base(const_cast<InvariantType*>(T), {}, {nullptr, -1, -1}))

#define ARITHMETIC_OP_FUNCS(T)                                                \
	ifunc("+",                                                                \
		  {tb::base(const_cast<InvariantType*>(T), {}, {nullptr, -1, -1})},   \
		  tb::base(const_cast<InvariantType*>(T), {}, {nullptr, -1, -1})),    \
		ifunc(                                                                \
			"-",                                                              \
			{tb::base(const_cast<InvariantType*>(T), {}, {nullptr, -1, -1})}, \
			tb::base(const_cast<InvariantType*>(T), {}, {nullptr, -1, -1})),  \
		ifunc(                                                                \
			"*",                                                              \
			{tb::base(const_cast<InvariantType*>(T), {}, {nullptr, -1, -1})}, \
			tb::base(const_cast<InvariantType*>(T), {}, {nullptr, -1, -1})),  \
		ifunc(                                                                \
			"/",                                                              \
			{tb::base(const_cast<InvariantType*>(T), {}, {nullptr, -1, -1})}, \
			tb::base(const_cast<InvariantType*>(T), {}, {nullptr, -1, -1})),  \
		ifunc(                                                                \
			"%",                                                              \
			{tb::base(const_cast<InvariantType*>(T), {}, {nullptr, -1, -1})}, \
			tb::base(const_cast<InvariantType*>(T), {}, {nullptr, -1, -1})),  \
		ifunc(                                                                \
			"**",                                                             \
			{tb::base(const_cast<InvariantType*>(T), {}, {nullptr, -1, -1})}, \
			tb::base(const_cast<InvariantType*>(T), {}, {nullptr, -1, -1})),  \
		ifunc(                                                                \
			"<=>",                                                            \
			{tb::base(const_cast<InvariantType*>(T), {}, {nullptr, -1, -1})}, \
			tb::base(const_cast<InvariantType*>(&T_INT), {},                  \
					 {nullptr, -1, -1})),                                     \
		ifunc(                                                                \
			"..",                                                             \
			{tb::base(const_cast<InvariantType*>(T), {}, {nullptr, -1, -1})}, \
			tb::base(const_cast<InvariantType*>(&T_RANGE),                    \
					 {tb::base(T, {}, {nullptr, -1, -1})},                    \
					 {nullptr, -1, -1})),                                     \
		ifunc(                                                                \
			"...",                                                            \
			{tb::base(const_cast<InvariantType*>(T), {}, {nullptr, -1, -1})}, \
			tb::base(const_cast<InvariantType*>(&T_RANGE),                    \
					 {tb::base(T, {}, {nullptr, -1, -1})}, {nullptr, -1, -1}))

static void addMembers(InvariantType* t,
					   std::initializer_list<Symbol*> members) {
	for (auto& m : members) t->addSymbol(m);
}

void initInvariantTypes() {
	T_ITERATOR.parentTypes.push_back(
		tb::base(const_cast<InvariantType*>(&T_ANY), {}, T_ANY.sourceMeta));
	T_ITERABLE.parentTypes.push_back(
		tb::base(const_cast<InvariantType*>(&T_ANY), {}, T_ANY.sourceMeta));
	T_RANGE.parentTypes.push_back(
		tb::base(const_cast<InvariantType*>(&T_ITERABLE),
				 {tb::base(T_RANGE.generics[0], {}, T_RANGE.sourceMeta)},
				 T_ITERABLE.sourceMeta));

	T_ITERATOR.addSymbol(
		func("next", TokenType::ID, {},
			 tb::base(T_ITERATOR.generics[0], {}, {nullptr, -1, -1})));
	T_ITERATOR.addSymbol(func("hasNext", TokenType::ID, {},
							  tb::base(&T_BOOL, {}, {nullptr, -1, -1})));
	T_RANGE.addSymbol(
		func("getStart", TokenType::ID, {},
			 tb::base(T_RANGE.generics[0], {}, {nullptr, -1, -1})));
	T_RANGE.addSymbol(
		func("getEnd", TokenType::ID, {},
			 tb::base(T_RANGE.generics[0], {}, {nullptr, -1, -1})));

	T_ITERABLE.addSymbol(
		func("iterator", TokenType::ID, {},
			 tb::base(&T_ITERATOR,
					  {tb::base(T_ITERABLE.generics[0], {}, {nullptr, -1, -1})},
					  {nullptr, -1, -1})));

	addMembers(&T_INT, {ARITHMETIC_OP_FUNCS(&T_INT), INT_OP_FUNCS(&T_INT)});
	addMembers(&T_INT8, {ARITHMETIC_OP_FUNCS(&T_INT8), INT_OP_FUNCS(&T_INT8)});
	addMembers(&T_INT16,
			   {ARITHMETIC_OP_FUNCS(&T_INT16), INT_OP_FUNCS(&T_INT16)});
	addMembers(&T_INT32,
			   {ARITHMETIC_OP_FUNCS(&T_INT32), INT_OP_FUNCS(&T_INT32)});
	addMembers(&T_INT64,
			   {ARITHMETIC_OP_FUNCS(&T_INT64), INT_OP_FUNCS(&T_INT64)});
	addMembers(&T_UINT, {ARITHMETIC_OP_FUNCS(&T_UINT), INT_OP_FUNCS(&T_UINT)});
	addMembers(&T_UINT8,
			   {ARITHMETIC_OP_FUNCS(&T_UINT8), INT_OP_FUNCS(&T_UINT8)});
	addMembers(&T_UINT16,
			   {ARITHMETIC_OP_FUNCS(&T_UINT16), INT_OP_FUNCS(&T_UINT16)});
	addMembers(&T_UINT32,
			   {ARITHMETIC_OP_FUNCS(&T_UINT32), INT_OP_FUNCS(&T_UINT32)});
	addMembers(&T_UINT64,
			   {ARITHMETIC_OP_FUNCS(&T_UINT64), INT_OP_FUNCS(&T_UINT64)});

	addMembers(&T_FLOAT, {ARITHMETIC_OP_FUNCS(&T_FLOAT)});
	addMembers(&T_DOUBLE, {ARITHMETIC_OP_FUNCS(&T_DOUBLE)});
	addMembers(&T_FLOAT80, {ARITHMETIC_OP_FUNCS(&T_FLOAT80)});
}
}  // namespace bt
}  // namespace acl