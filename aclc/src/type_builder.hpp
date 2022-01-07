#pragma once

#include <initializer_list>

#include "ast.hpp"

namespace acl {
namespace tb {
TypeRef* base(Type* referent, const List<TypeRef*>& generics,
			  const SourceMeta& meta);
TypeRef* optional(TypeRef* content);
TypeRef* unwrappedOptional(TypeRef* content);
TypeRef* pointer(TypeRef* content);
TypeRef* array(TypeRef* content);
TypeRef* map(TypeRef* key, TypeRef* value);
TypeRef* tuple(std::initializer_list<TypeRef*> types);
TypeRef* tuple(const List<TypeRef*>& types);
FunctionTypeRef* function(std::initializer_list<TypeRef*> paramTypes,
						  TypeRef* returnType);
FunctionTypeRef* function(const List<TypeRef*>& paramTypes,
						  TypeRef* returnType);
}  // namespace tb
}  // namespace acl