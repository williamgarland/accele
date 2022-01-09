#pragma once

#include <filesystem>

#include "ast.hpp"
#include "common.hpp"

namespace acl {
class ImportHandler {
	CompilerContext& ctx;
	Ast* ast;

	void resolveImport(Import* i);
	Ast* resolveImportSource(ImportSource* src);
	Ast* resolveImportSourcePath(const std::filesystem::path& path,
								 const SourceMeta& meta);
	Ast* compileImport(const std::filesystem::path& path,
					   const SourceMeta& meta);
	void getPossibleBaseDirs(bool declaredRelative,
							 List<std::filesystem::path>& dest);

   public:
	ImportHandler(CompilerContext& ctx, Ast* ast);
	void resolveImports();
};
}  // namespace acl