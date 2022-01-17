#pragma once

#include <cstdint>
#include <filesystem>
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

struct ModuleInfo {
	String dir;
	String path;
	String name;
};

using FilePos = std::intmax_t;

struct SourceMeta {
	const ModuleInfo* moduleInfo;
	FilePos pos;
	int line;
	int col;
};

struct Ast;

struct Module {
	ModuleInfo moduleInfo;
	Ast* ast;
	List<String> source;

	Module(const ModuleInfo& moduleInfo, Ast* ast, const List<String>& source);
	~Module();
};

// Contains common flags and features to be used across all parts of the
// compilation process.
struct CompilerContext {
	Map<int, bool> warnings;
	List<Module*> modules;
	List<std::filesystem::path> additionalImportDirs;
	std::filesystem::path globalImportDir;
	CompilerContext();
};

template <typename T>
bool listContains(const List<T>& list, const T& t) {
	for (const auto& e : list)
		if (e == t) return true;
	return false;
}
}  // namespace acl