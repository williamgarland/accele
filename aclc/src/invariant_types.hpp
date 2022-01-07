#pragma once

#include <initializer_list>

#include "ast.hpp"

/*
This file defines the built-in types which are always guaranteed to be available
across all modules. Even when the "--no-builtins" flag or the "@nobuiltins" tag
are used, the invariant types will still be available. This is necessary because
without these types, the language cannot function as intended.
*/

namespace acl {
namespace bt {
class InvariantType : public Type, public Scope {
   public:
	InvariantType(const String& id,
				  std::initializer_list<TypeRef*> parentTypes);
	virtual ~InvariantType();
	virtual void toJson(StringBuffer& dest) const override;
};

extern const InvariantType* ANY;
extern const InvariantType* NUMBER;
extern const InvariantType* INT;
extern const InvariantType* INT8;
extern const InvariantType* INT16;
extern const InvariantType* INT32;
extern const InvariantType* INT64;
extern const InvariantType* UINT;
extern const InvariantType* UINT8;
extern const InvariantType* UINT16;
extern const InvariantType* UINT32;
extern const InvariantType* UINT64;
extern const InvariantType* FLOAT;
extern const InvariantType* DOUBLE;
extern const InvariantType* FLOAT80;
extern const InvariantType* BOOL;
extern const InvariantType* STRING;
extern const InvariantType* VOID;
extern const InvariantType* ARRAY;
extern const InvariantType* MAP;
extern const InvariantType* TUPLE;
extern const InvariantType* FUNCTION;
extern const InvariantType* OPTIONAL;
extern const InvariantType* UNWRAPPED_OPTIONAL;
extern const InvariantType* POINTER;

bool isInvariantType(const String& id);
const InvariantType* resolveInvariantType(const Token* id);
}  // namespace bt
}  // namespace acl