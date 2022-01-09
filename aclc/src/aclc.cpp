#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <initializer_list>
#include <iostream>

#include "exceptions.hpp"
#include "lexer.hpp"
#include "parser.hpp"
#include "resolver.hpp"

#define ACLC_VERSION "1.0.0a"

#define ACLC_GLOBAL_IMPORT_DIR "lib/packages"

#define ACLC_VERSION_MESSAGE "Accele Compiler " ACLC_VERSION

#define ACLC_USAGE_MESSAGE "Usage: aclc [options] file..."

#define ACLC_HELP_MESSAGE                                                      \
	"The Accele Compiler (ACLC) is used to compile Accele source "             \
	"files.\n\n" ACLC_USAGE_MESSAGE                                            \
	"\n\n"                                                                     \
	"Options:\n"                                                               \
	"    -a, --arch <arch>                            Specify the target "     \
	"architecture\n"                                                           \
	"    -c, --cpp-compiler <compiler>                Specify the target C++ " \
	"compiler\n"                                                               \
	"    -C, --custom-cpp-compiler <path>             Specify the path to "    \
	"the "                                                                     \
	"JSON file "                                                               \
	"detailing the custom C++ compiler to use\n"                               \
	"    --dump-ast <path>                            Specify the directory "  \
	"to "                                                                      \
	"store the generated module ASTs\n"                                        \
	"    -G, --global-dir <path>                      Specify the path to "    \
	"the "                                                                     \
	"global import "                                                           \
	"directory\n"                                                              \
	"    -h, --help                                   Output help "            \
	"information\n"                                                            \
	"    -I, --import-dir <path>                      Specify additional "     \
	"import directory\n"                                                       \
	"    -o, --output-dest <path>                     Specify the output "     \
	"file "                                                                    \
	"or directory\n"                                                           \
	"    -p, --platform <platform>                    Specify the platform "   \
	"to "                                                                      \
	"target\n"                                                                 \
	"    -t, --target <target>                        Specify the output "     \
	"type\n"                                                                   \
	"    -v, --version                                Output the compiler "    \
	"version\n"                                                                \
	"    -V, --verbose                                Output verbose "         \
	"information during compilation"

#define ACLC_BASIC_INFO ACLC_VERSION_MESSAGE "\n" ACLC_USAGE_MESSAGE

#if defined(MSVC) || defined(_MSC_VER)
#if defined(_M_X64) || defined(_M_AMD64)
#define THIS_ARCH "x86_64"
#else
#if defined(_M_X86) || defined(_M_IX86)
#define THIS_ARCH "x86"
#endif
#endif

#if defined(_M_ARM64)
#define THIS_ARCH "arm64"
#else
#if defined(_M_ARM)
#define THIS_ARCH "arm"
#endif
#endif
#endif

#if defined(GCC) || defined(__GNUC__)
#if defined(__amd64__) || defined(__amd64) || defined(__x86_64__) || \
	defined(__x86_64)
#define THIS_ARCH "x86_64"
#else
#if defined(i386) || defined(__i386) || defined(__i386__) || \
	defined(__i486__) || defined(__i586__) || defined(__i686__)
#define THIS_ARCH "x86"
#endif
#endif

#if defined(__aarch64__)
#define THIS_ARCH "arm64"
#else
#if defined(__arm__)
#define THIS_ARCH "arm"
#endif
#endif
#endif

#if __clang__
#if defined(__amd64) || defined(__amd64__) || defined(__x86_64) || \
	defined(__x86_64__)
#define THIS_ARCH "x86_64"
#else
#if defined(__x86) || defined(__x86__)
#define THIS_ARCH "x86"
#endif
#endif

#if defined(__aarch64__)
#define THIS_ARCH "arm64"
#else
#if defined(__arm__)
#define THIS_ARCH "arm"
#endif
#endif
#endif

#ifndef THIS_ARCH
#define THIS_ARCH "unknown"
#endif

#if defined(_WIN32) || defined(_WIN64) || defined(__CYGWIN__)
#define THIS_PLATFORM Platform::WINDOWS
#else
#if defined(__APPLE__) || defined(__MACH__)
#define THIS_PLATFORM Platform::MACOS
#else
#if defined(unix) || defined(__unix) || defined(__unix__)
#define THIS_PLATFORM Platform::LINUX
#else
#define THIS_PLATFORM Platform::UNKNOWN
#endif
#endif
#endif

/*
--dump-ast <dest> = Dump the AST of the input modules to the specified
directory. Each module AST will be dumped into a JSON file with the filename
format "<module_name>.ast.json".

-o, --output-dest <dest> = Specify the output destination. Whether this should
be a file or a directory depends on the output type (specified by "-t").

-G, --global-dir <dir> = Specify the global import directory. By default, the
compiler will obtain this directory based on the "ACCELE_HOME" environment
variable, however specifying this option will override that. The global import
directory will always be searched last.

-I, --import-dir <dir> = Specify additional import directories. You can specify
any number of additional import directories. The order in which these
directories will be searched during module compilation is arbitrary.

-a, --arch <arch> = Specify the target architecture. If the architecture is not
specified, it will be whatever the machine is that is running the compiler.

Available architectures:
x86 - 32-bit Intel/AMD (x86-based)
x86_64 - 64-bit Intel/AMD (x86-based)
arm - 32-bit ARM
arm64 - 64-bit ARM

-p, --platform <platform> = Specify the target platform. This determines which
underlying C++ compiler is used. For Windows, it's Visual C++, for MacOS, it's
Clang, and for Linux, it's GCC. If you want to hand-specify a compiler, you can
use the "-c"/"--cpp-compiler" option. If the platform is not specified, it will
be whatever the machine is that is running the compiler.

Available platforms:
windows, win - Windows
macos, osx - MacOS
linux - Linux

-t, --target <target> = Specify the output type.

Available targets:
exec - Binary executable. This is the default option.
dlib, dynamic_lib - Dynamically-linked library.
slib, static_lib - Statically-linked library.
def - Module definition files.
cpp - C++ header and source files.
obj - Object files.

-c, --cpp-compiler <compiler> = Specify the compiler to use when compiling the
generated C++ code. This option only accepts one of "msvc", "clang", or
"gcc". If you want to use a different compiler, either generate the C++ source
files using "-t cpp" and pass it to your compiler or use the "-C" option.

-C, --custom-cpp-compiler <compiler-script> = Specify a path to a JSON file
detailing a custom compiler to use when compiling the generated C++ code. The
JSON file should be in the following format:

{
	"version": "1.0.0",
	"command": [
		"<path-to-compiler-executable>",
		"argument1",
		"${src}",
		"argument2",
		"${dest}"
	]
}

Macros, which are identifiers surrounded by "${}", allow you to inject
information from ACLC into your custom compiler. The possible macros that can be
inserted into the command arguments are:

${srcDir} - The directory containing the C++ source files.
${src} - The list of C++ source files.
${headerDir} - The directory containing the C++ header files.
${headers} - The list of C++ header files.
${dest} - The target file or directory as specified in the provided arguments to
ACLC.
${arch} - The target architecture.
${platform} - The target platform.
${target} - The output type.

-h, --help = Display help information. If this argument is specified multiple
times, the help information will only be displayed once.

-v, --version = Display the compiler version. If this argument is specified
multiple times, the version information will only be displayed once.

-V, --verbose = Display verbose information during compilation.

*/

namespace {
class ArgumentException : public std::exception {
	acl::String message;

   public:
	ArgumentException(const acl::String& message) : message(message) {}
	virtual ~ArgumentException() {}
	virtual const char* what() const noexcept { return message.c_str(); }
};

struct CppCompiler {
	static acl::List<CppCompiler*> compilers;

	acl::List<acl::String> strings;

	CppCompiler(std::initializer_list<acl::String> strings) {
		this->strings.insert(this->strings.end(), strings.begin(),
							 strings.end());
		compilers.push_back(this);
	}

	static const CppCompiler GCC;
	static const CppCompiler CLANG;
	static const CppCompiler MSVC;

	static const CppCompiler& getCppCompiler(const acl::String& str) {
		for (const auto& p : compilers) {
			for (const auto& s : p->strings) {
				if (s == str) return *p;
			}
		}
		acl::StringBuffer sb;
		sb << "Invalid C++ compiler \"" << str << "\"";
		throw ArgumentException(sb.str());
	}
};

acl::List<CppCompiler*> CppCompiler::compilers = {};

const CppCompiler CppCompiler::GCC = CppCompiler({"gcc"});
const CppCompiler CppCompiler::CLANG = CppCompiler({"clang"});
const CppCompiler CppCompiler::MSVC = CppCompiler({"msvc"});

struct Platform {
	static acl::List<Platform*> platforms;

	acl::List<acl::String> strings;
	const CppCompiler& defaultCompiler;

	Platform(const CppCompiler& defaultCompiler,
			 std::initializer_list<acl::String> strings)
		: defaultCompiler(defaultCompiler) {
		this->strings.insert(this->strings.end(), strings.begin(),
							 strings.end());
		platforms.push_back(this);
	}

	static const Platform WINDOWS;
	static const Platform MACOS;
	static const Platform LINUX;
	static const Platform UNKNOWN;

	static const Platform& getPlatform(const acl::String& str) {
		for (const auto& p : platforms) {
			for (const auto& s : p->strings) {
				if (s == str) return *p;
			}
		}
		acl::StringBuffer sb;
		sb << "Invalid platform \"" << str << "\"";
		throw ArgumentException(sb.str());
	}
};

acl::List<Platform*> Platform::platforms = {};

const Platform Platform::WINDOWS =
	Platform(CppCompiler::MSVC, {"windows", "win"});
const Platform Platform::MACOS = Platform(CppCompiler::CLANG, {"macos", "osx"});
const Platform Platform::LINUX = Platform(CppCompiler::GCC, {"linux"});
const Platform Platform::UNKNOWN = Platform(CppCompiler::GCC, {});

struct OutputTarget {
	static acl::List<OutputTarget*> targets;

	acl::List<acl::String> strings;

	OutputTarget(std::initializer_list<acl::String> strings) {
		this->strings.insert(this->strings.end(), strings.begin(),
							 strings.end());
		targets.push_back(this);
	}

	static const OutputTarget EXEC;
	static const OutputTarget SLIB;
	static const OutputTarget DLIB;
	static const OutputTarget CPP;
	static const OutputTarget DEF;
	static const OutputTarget OBJ;

	static const OutputTarget& getOutputTarget(const acl::String& str) {
		for (const auto& p : targets) {
			for (const auto& s : p->strings) {
				if (s == str) return *p;
			}
		}
		acl::StringBuffer sb;
		sb << "Invalid output target \"" << str << "\"";
		throw ArgumentException(sb.str());
	}
};

acl::List<OutputTarget*> OutputTarget::targets = {};

const OutputTarget OutputTarget::EXEC = OutputTarget({"exec", "exe"});
const OutputTarget OutputTarget::SLIB =
	OutputTarget({"slib", "static_lib", "lib", "a"});
const OutputTarget OutputTarget::DLIB =
	OutputTarget({"dlib", "dylib", "dynamic_lib", "dll", "so"});
const OutputTarget OutputTarget::CPP = OutputTarget({"cpp", "c++"});
const OutputTarget OutputTarget::DEF = OutputTarget({"def", "acldef"});
const OutputTarget OutputTarget::OBJ = OutputTarget({"obj", "o"});

void displayBasicInfo();
void displayVersion();
void displayHelp();
void displayUsage();
void setOutputDest(const acl::String& dest);
void enableDumpAst(const acl::String& dest);
void setGlobalImportDir(const acl::String& dir);
void addImportDir(const acl::String& dir);
void setArch(const acl::String& arch);
void setPlatform(const Platform& platform);
void setTarget(const OutputTarget& target);
void setCppCompiler(const CppCompiler& compiler);
void addInputFile(const acl::String& file);
void parseArgs(int argc, char* argv[]);
void compile();
void compileModule(acl::CompilerContext& ctx,
				   const std::filesystem::path& path);
void dumpAst(const acl::Ast* ast, const std::filesystem::path& destDir);
void setDefaultGlobalImportDir();

struct AclcOptions {
	const OutputTarget* target = &OutputTarget::EXEC;
	const Platform* platform = &THIS_PLATFORM;
	acl::String arch = THIS_ARCH;
	const CppCompiler* cppCompiler = &THIS_PLATFORM.defaultCompiler;
	std::filesystem::path globalImportPath;
	bool explicitGlobalImportPath = false;
	acl::List<std::filesystem::path> additionalImportPaths;
	acl::List<std::filesystem::path> inputModules;
	std::filesystem::path outputDest;
	std::filesystem::path astDest;
	bool dumpAst = false;
	bool verbose = false;
};

AclcOptions compilerOptions;
}  // namespace

int main(int argc, char* argv[]) {
	using namespace acl;

	if (argc < 2) {
		displayBasicInfo();
		return 0;
	}

	try {
		parseArgs(argc, argv);
	} catch (ArgumentException& e) {
		acl::log::error(e.what());
		return 1;
	}

	return 0;
}

namespace {
void parseArgs(int argc, char* argv[]) {
	bool foundHelp = false;
	bool foundVersion = false;
	bool runCompiler = true;
	for (int i = 1; i < argc; i++) {
		if ((strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) &&
			!foundHelp) {
			foundHelp = true;
			displayHelp();
			runCompiler = false;
		} else if ((strcmp(argv[i], "-v") == 0 ||
					strcmp(argv[i], "--version") == 0) &&
				   !foundVersion) {
			foundVersion = true;
			displayVersion();
			runCompiler = false;
		} else if (strcmp(argv[i], "-G") == 0 ||
				   strcmp(argv[i], "--global-dir") == 0) {
			if (i + 1 >= argc)
				throw ArgumentException(
					"Expected directory following \"-G\" or \"--global-dir\" "
					"option");
			setGlobalImportDir(argv[i + 1]);
			i++;
		} else if (strcmp(argv[i], "-I") == 0 ||
				   strcmp(argv[i], "--import-dir") == 0) {
			if (i + 1 >= argc)
				throw ArgumentException(
					"Expected directory following \"-I\" or \"--import-dir\" "
					"option");
			addImportDir(argv[i + 1]);
			i++;
		} else if (strcmp(argv[i], "-a") == 0 ||
				   strcmp(argv[i], "--arch") == 0) {
			if (i + 1 >= argc)
				throw ArgumentException(
					"Expected architecture following \"-a\" or \"--arch\" "
					"option");
			setArch(argv[i + 1]);
			i++;
		} else if (strcmp(argv[i], "-p") == 0 ||
				   strcmp(argv[i], "--platform") == 0) {
			if (i + 1 >= argc)
				throw ArgumentException(
					"Expected platform following \"-p\" or \"--platform\" "
					"option");
			setPlatform(Platform::getPlatform(argv[i + 1]));
			i++;
		} else if (strcmp(argv[i], "-t") == 0 ||
				   strcmp(argv[i], "--target") == 0) {
			if (i + 1 >= argc)
				throw ArgumentException(
					"Expected target following \"-t\" or \"--target\" "
					"option");
			setTarget(OutputTarget::getOutputTarget(argv[i + 1]));
			i++;
		} else if (strcmp(argv[i], "-c") == 0 ||
				   strcmp(argv[i], "--cpp-compiler") == 0) {
			if (i + 1 >= argc)
				throw ArgumentException(
					"Expected compiler following \"-c\" or \"--cpp-compiler\" "
					"option");
			setCppCompiler(CppCompiler::getCppCompiler(argv[i + 1]));
			i++;
		} else if (strcmp(argv[i], "-o") == 0 ||
				   strcmp(argv[i], "--output-dest") == 0) {
			if (i + 1 >= argc)
				throw ArgumentException(
					"Expected output destination following \"-o\" or "
					"\"--output-dest\" "
					"option");
			setOutputDest(argv[i + 1]);
			i++;
		} else if (strcmp(argv[i], "--dump-ast") == 0) {
			if (i + 1 >= argc)
				throw ArgumentException(
					"Expected output destination following \"--dump-ast\" "
					"option");
			enableDumpAst(argv[i + 1]);
			i++;
		} else if (strcmp(argv[i], "-V") == 0 ||
				   strcmp(argv[i], "--verbose") == 0) {
			compilerOptions.verbose = true;
		} else if (strlen(argv[i]) >= 1 && argv[i][0] == '-') {
			acl::StringBuffer sb;
			sb << "Invalid argument \"" << argv[i] << "\"";
			throw ArgumentException(sb.str());
		} else {
			addInputFile(argv[i]);
		}
	}

	if (runCompiler) {
		if (compilerOptions.inputModules.empty()) {
			acl::log::error("No input modules provided");
			displayUsage();
			exit(1);
		}

		if (!compilerOptions.explicitGlobalImportPath) {
			setDefaultGlobalImportDir();
		}

		compile();
	}
}

void compile() {
	using namespace acl;
	CompilerContext ctx;

	for (auto& p : compilerOptions.additionalImportPaths)
		ctx.additionalImportDirs.push_back(p);

	ctx.globalImportDir = compilerOptions.globalImportPath;

	for (const auto& p : compilerOptions.inputModules) {
		bool shouldCompile = true;
		for (const auto& m : ctx.modules) {
			if (p == m->moduleInfo.path) {
				shouldCompile = false;
				break;
			}
		}

		if (shouldCompile) {
			compileModule(ctx, p);
		}
	}
}

void compileModule(acl::CompilerContext& ctx,
				   const std::filesystem::path& path) {
	std::ifstream ifs(path);

	if (!ifs) throw ArgumentException("Invalid input module");

	acl::StringBuffer sb;
	sb << ifs.rdbuf();
	ifs.close();

	// TODO: A .accele file and a .acldef file should be parsed and resolved
	// differently. Also, .acldef files cannot be translated to C++ source or
	// OBJ files. They can only be used to reference a library.

	acl::Parser parser = acl::Parser(ctx, acl::Lexer(path.string(), sb));
	auto ast = parser.parse();

	ctx.modules.push_back(ast);

	// TODO: Handle the rest of compilation here

	if (compilerOptions.dumpAst) {
		dumpAst(ast, compilerOptions.astDest);
	}
}

void setDefaultGlobalImportDir() {
	auto acceleHome = getenv("ACCELE_HOME");

	if (!acceleHome) {
		acl::log::warn(
			"No \"ACCELE_HOME\" environment variable found. This may cause "
			"problems for global imports.");
		return;
	}

	std::filesystem::path p =
		std::filesystem::path(acceleHome) / ACLC_GLOBAL_IMPORT_DIR;

	setGlobalImportDir(p.string());
}

void displayBasicInfo() { std::cout << ACLC_BASIC_INFO << "\n"; }

void displayVersion() { std::cout << ACLC_VERSION_MESSAGE << "\n"; }

void displayHelp() { std::cout << ACLC_HELP_MESSAGE << "\n"; }

void displayUsage() { std::cout << ACLC_USAGE_MESSAGE << "\n"; }

void setOutputDest(const acl::String& dest) {
	compilerOptions.outputDest = dest;
}

void enableDumpAst(const acl::String& dest) {
	std::filesystem::path p = dest;
	if (!std::filesystem::exists(p)) {
		acl::StringBuffer sb;
		sb << "The specified AST dump directory \"" << dest
		   << "\" does not exist";
		throw ArgumentException(sb.str());
	}

	if (!std::filesystem::is_directory(p)) {
		acl::StringBuffer sb;
		sb << "The specified AST dump directory \"" << dest
		   << "\" is not a directory";
		throw ArgumentException(sb.str());
	}

	compilerOptions.dumpAst = true;
	compilerOptions.astDest = p;
}

void setGlobalImportDir(const acl::String& dir) {
	std::filesystem::path p = dir;
	if (!std::filesystem::exists(p)) {
		acl::StringBuffer sb;
		sb << "The specified global import directory \"" << dir
		   << "\" does not exist";
		throw ArgumentException(sb.str());
	}

	if (!std::filesystem::is_directory(p)) {
		acl::StringBuffer sb;
		sb << "The specified global import directory \"" << dir
		   << "\" is not a directory";
		throw ArgumentException(sb.str());
	}

	compilerOptions.globalImportPath = p;
	compilerOptions.explicitGlobalImportPath = true;
}

void addImportDir(const acl::String& dir) {
	std::filesystem::path p = dir;
	if (!std::filesystem::exists(p)) {
		acl::StringBuffer sb;
		sb << "The specified import directory \"" << dir << "\" does not exist";
		throw ArgumentException(sb.str());
	}

	if (!std::filesystem::is_directory(p)) {
		acl::StringBuffer sb;
		sb << "The specified import directory \"" << dir
		   << "\" is not a directory";
		throw ArgumentException(sb.str());
	}
	compilerOptions.additionalImportPaths.push_back(p);
}

void setArch(const acl::String& arch) { compilerOptions.arch = arch; }

void setPlatform(const Platform& platform) {
	compilerOptions.platform = &platform;
}

void setTarget(const OutputTarget& target) { compilerOptions.target = &target; }

void setCppCompiler(const CppCompiler& compiler) {
	compilerOptions.cppCompiler = &compiler;
}

void addInputFile(const acl::String& file) {
	std::filesystem::path p = file;
	if (!std::filesystem::exists(p)) {
		acl::StringBuffer sb;
		sb << "The specified input module \"" << file << "\" does not exist";
		throw ArgumentException(sb.str());
	}

	if (std::filesystem::is_directory(p)) {
		acl::StringBuffer sb;
		sb << "The specified input module \"" << file << "\" is not a file";
		throw ArgumentException(sb.str());
	}
	compilerOptions.inputModules.push_back(p);
}

void dumpAst(const acl::Ast* ast, const std::filesystem::path& destDir) {
	auto destFile = destDir / (ast->moduleInfo.name + ".ast.json");
	std::ofstream ofs(destFile);

	if (!ofs) {
		acl::StringBuffer sb;
		sb << "Failed to write AST for destination file \"" << destFile.string()
		   << "\"";
		acl::log::error(sb.str());
	}

	acl::StringBuffer sb;
	ast->globalScope->toJson(sb);
	auto str = sb.str();
	ofs << str;
}
}  // namespace