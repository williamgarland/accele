#pragma once

#include <filesystem>

#include "ast.hpp"
#include "common.hpp"

namespace acl {
class ImportHandler {
	CompilerContext& ctx;
	Module* mod;

	void resolveImport(Import* i);
	Ast* resolveImportSource(ImportSource* src);
	Ast* resolveImportSourcePath(const std::filesystem::path& path,
								 const SourceMeta& meta);
	Ast* compileImport(const std::filesystem::path& path,
					   const SourceMeta& meta);
	void getPossibleBaseDirs(bool declaredRelative,
							 List<std::filesystem::path>& dest);
	void resolveImportTarget(Ast* searchTarget, ImportTarget* target,
							 const List<ImportTarget*>& allTargets);

   public:
	ImportHandler(CompilerContext& ctx, Module* mod);
	void resolveImports();
};
}  // namespace acl