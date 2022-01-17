#pragma once

#include <cstdint>

#include "objref.hpp"

namespace acl {
class Any;

struct RefCount {
	uint8_t origin;
	int* count;
	RefCount();
	~RefCount();
	RefCount& operator++();
	RefCount& operator--();
	bool operator==(int other);
	operator int() const;
	void useTable(Any* obj);
};

class Type;
struct String;

class Any {
   private:
	Type* type;

   public:
	RefCount rcount;
	virtual ~Any();
	virtual StrongRef<String> asString() const;

	static Type _TYPE;
};

int getRefTableCount(Any* obj);
}  // namespace acl