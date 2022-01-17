#include "../include/objbase.hpp"

#include <sstream>
#include <unordered_map>

#include "../include/aclrt.hpp"

namespace {
std::unordered_map<acl::Any*, int> refTable;
}

#define ORIGIN_SELF 0
#define ORIGIN_TABLE 1

namespace acl {
int getRefTableCount(Any* obj) { return refTable.at(obj); }

RefCount::RefCount() : origin(ORIGIN_SELF), count(new int(0)) {}

RefCount::~RefCount() {
	if (origin == ORIGIN_SELF) delete count;
}

RefCount& RefCount::operator++() {
	(*count)++;
	return *this;
}

RefCount& RefCount::operator--() {
	(*count)--;
	return *this;
}

bool RefCount::operator==(int other) { return *count == other; }

RefCount::operator int() const { return *count; }

void RefCount::useTable(Any* obj) {
	refTable[obj] = *count;
	if (origin == ORIGIN_SELF) delete count;
	count = &refTable.at(obj);
}

Any::~Any() {}

StrongRef<String> Any::asString() const {
	std::stringstream ss;
	ss << "Any@" << std::hex << (puint64)this;
	pstring str = ss.str();
	return StrongRef<String>(new String(str));
}
}  // namespace acl