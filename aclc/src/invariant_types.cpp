#include "invariant_types.hpp"

#include "exceptions.hpp"
#include "type_builder.hpp"

namespace acl {
namespace bt {
InvariantType::InvariantType(const String& id,
							 std::initializer_list<TypeRef*> parentTypes)
	: Type(new Token(TokenType::ID, id, {"<bt>", -1, -1}), {}), Scope(nullptr) {
	this->parentTypes.insert(this->parentTypes.end(), parentTypes.begin(),
							 parentTypes.end());
}

InvariantType::~InvariantType() {
	for (auto& t : parentTypes) {
		delete t;
	}
}

void InvariantType::toJson(StringBuffer& dest) const {
	dest << "{\n\"name\": \"InvariantType\",\n\"id\": \"" << this->id->data
		 << "\"\n}";
}

static const InvariantType T_ANY = InvariantType("Any", {});
static const InvariantType T_NUMBER = InvariantType(
	"Number",
	{tb::base(const_cast<InvariantType*>(&T_ANY), {}, T_ANY.sourceMeta)});
static const InvariantType T_INT = InvariantType(
	"Int",
	{tb::base(const_cast<InvariantType*>(&T_NUMBER), {}, T_NUMBER.sourceMeta)});
static const InvariantType T_INT8 = InvariantType(
	"Int8",
	{tb::base(const_cast<InvariantType*>(&T_NUMBER), {}, T_NUMBER.sourceMeta)});
static const InvariantType T_INT16 = InvariantType(
	"Int16",
	{tb::base(const_cast<InvariantType*>(&T_NUMBER), {}, T_NUMBER.sourceMeta)});
static const InvariantType T_INT32 = InvariantType(
	"Int32",
	{tb::base(const_cast<InvariantType*>(&T_NUMBER), {}, T_NUMBER.sourceMeta)});
static const InvariantType T_INT64 = InvariantType(
	"Int64",
	{tb::base(const_cast<InvariantType*>(&T_NUMBER), {}, T_NUMBER.sourceMeta)});
static const InvariantType T_UINT = InvariantType(
	"UInt",
	{tb::base(const_cast<InvariantType*>(&T_NUMBER), {}, T_NUMBER.sourceMeta)});
static const InvariantType T_UINT8 = InvariantType(
	"UInt8",
	{tb::base(const_cast<InvariantType*>(&T_NUMBER), {}, T_NUMBER.sourceMeta)});
static const InvariantType T_UINT16 = InvariantType(
	"UInt16",
	{tb::base(const_cast<InvariantType*>(&T_NUMBER), {}, T_NUMBER.sourceMeta)});
static const InvariantType T_UINT32 = InvariantType(
	"UInt32",
	{tb::base(const_cast<InvariantType*>(&T_NUMBER), {}, T_NUMBER.sourceMeta)});
static const InvariantType T_UINT64 = InvariantType(
	"UInt64",
	{tb::base(const_cast<InvariantType*>(&T_NUMBER), {}, T_NUMBER.sourceMeta)});
static const InvariantType T_FLOAT = InvariantType(
	"Float",
	{tb::base(const_cast<InvariantType*>(&T_NUMBER), {}, T_NUMBER.sourceMeta)});
static const InvariantType T_DOUBLE = InvariantType(
	"Double",
	{tb::base(const_cast<InvariantType*>(&T_NUMBER), {}, T_NUMBER.sourceMeta)});
static const InvariantType T_FLOAT80 = InvariantType(
	"Float80",
	{tb::base(const_cast<InvariantType*>(&T_NUMBER), {}, T_NUMBER.sourceMeta)});
static const InvariantType T_BOOL = InvariantType(
	"Bool",
	{tb::base(const_cast<InvariantType*>(&T_ANY), {}, T_ANY.sourceMeta)});
static const InvariantType T_STRING = InvariantType(
	"String",
	{tb::base(const_cast<InvariantType*>(&T_ANY), {}, T_ANY.sourceMeta)});
static const InvariantType T_VOID =
	InvariantType("Void", {});	// TODO: Not sure whether this needs to have a
								// parent type of "Any"...
static const InvariantType T_ARRAY = InvariantType(
	"Array",
	{tb::base(const_cast<InvariantType*>(&T_ANY), {}, T_ANY.sourceMeta)});
static const InvariantType T_MAP = InvariantType(
	"Map",
	{tb::base(const_cast<InvariantType*>(&T_ANY), {}, T_ANY.sourceMeta)});
static const InvariantType T_TUPLE = InvariantType(
	"Tuple",
	{tb::base(const_cast<InvariantType*>(&T_ANY), {}, T_ANY.sourceMeta)});
static const InvariantType T_FUNCTION = InvariantType(
	"Function",
	{tb::base(const_cast<InvariantType*>(&T_ANY), {}, T_ANY.sourceMeta)});
static const InvariantType T_OPTIONAL = InvariantType(
	"Optional",
	{tb::base(const_cast<InvariantType*>(&T_ANY), {}, T_ANY.sourceMeta)});
static const InvariantType T_UNWRAPPED_OPTIONAL = InvariantType(
	"UnwrappedOptional",
	{tb::base(const_cast<InvariantType*>(&T_ANY), {}, T_ANY.sourceMeta)});
static const InvariantType T_POINTER = InvariantType(
	"Pointer",
	{tb::base(const_cast<InvariantType*>(&T_ANY), {}, T_ANY.sourceMeta)});

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

static constexpr int T_INVARIANTS_LEN = 25;

static const InvariantType* T_INVARIANTS[T_INVARIANTS_LEN] = {
	ANY,	NUMBER, INT,	 INT8,	   INT16,	 INT32,
	INT64,	UINT,	UINT8,	 UINT16,   UINT32,	 UINT64,
	FLOAT,	DOUBLE, FLOAT80, BOOL,	   STRING,	 VOID,
	ARRAY,	MAP,	TUPLE,	 FUNCTION, OPTIONAL, UNWRAPPED_OPTIONAL,
	POINTER};

bool isInvariantType(const String& id) {
	for (int i = 0; i < T_INVARIANTS_LEN; i++)
		if (T_INVARIANTS[i]->id->data == id) return true;
	return false;
}

const InvariantType* resolveInvariantType(const Token* id) {
	for (int i = 0; i < T_INVARIANTS_LEN; i++)
		if (T_INVARIANTS[i]->id->data == id->data) return T_INVARIANTS[i];
	throw AclException(ASP_CORE_UNKNOWN, id->meta, "Unresolved symbol");
}
}  // namespace bt
}  // namespace acl