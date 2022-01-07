#pragma once

#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

namespace acl {
typedef std::string String;
typedef std::stringstream StringBuffer;

template <typename T>
using List = std::vector<T>;

template <typename K, typename V>
using Map = std::unordered_map<K, V>;

struct SourceMeta {
	String file;
	int line;
	int col;
};

// Contains common flags and features to be used across all parts of the
// compilation process.
struct CompilerContext {
	Map<String, bool> warnings;
	CompilerContext();
};

template <typename T>
bool listContains(const List<T>& list, const T& t) {
	for (const auto& e : list)
		if (e == t) return true;
	return false;
}
}  // namespace acl