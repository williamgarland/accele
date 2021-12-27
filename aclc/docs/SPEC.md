This document defines the formal specifications for Accele.

Accele Specification Protocol - A formal way of describing a feature of Accele. ASPs dictate how the language works as well as best practices and style suggestions.

ASPs are written using the format `V.C.N` where `V` is the major language version, `C` is the protocol category, and `N` is the protocol number within the category.
Multiple features should not be included within a single protocol. 

## ASP Categories
| Number | Title | Description |
|:------:|-------|-------------|
| 0      | Core  | A core feature of the language. ASPs of this category are requirements, not suggestions, and must be followed in order to produce working code. If code does not adhere to a protocol in this category, the compiler will generate an error. |
| 1      | Recommendation | A recommendation for how code should be written. If code does not adhere to a protocol in this category, the compiler will generate a warning. |
| 2      | Suggestion | A suggestion for how code should be written. This includes things like variable naming and importing, among other things. The compiler will not warn you if your code does not follow these protocols. |

## Protocols
### 1.0.1 - Integer literals
Integer literals can be written in four different bases: base-16, base-10, base-8, and base-2. Base-10 is the default, written as a sequence of one or more of the digits 0-9. Base-16 is written with a preceeding `0x` or `0X`, followed by one or more hexadecimal digits. Base-8 is written with a preceeding `0o` or `0O`, followed by one or more octal digits. Base-2 is written with a preceeding `0b` or `0B`, followed by one or more binary digits. Unless otherwise casted, all integer literals are assumed to be the `Int` type.

#### Grammar
| Type | Grammar |
|:----:|---------|
| `INTEGER_LITERAL` | `/[0-9]+/` |
| `HEX_LITERAL` | `/0[xX][0-9a-fA-F]+/` |
| `OCTAL_LITERAL` | `/0[oO][0-7]+/` |
| `BINARY_LITERAL` | `/0[bB][01]+/` |

### 1.0.2 - Floating-point literals
Floating-point literals are written using base-10 in either decimal form, exponential form, or a combination of the two. The mantissa is specified using base-10 with a dot (`.`) as the separator, and the exponent is specified with a preceeding `e` or `E` followed by a positive or negative base-10 integer.

#### Grammar
```
FloatLiteral = (Digit+ '.' Digit* Exponent?) | ('.'? Digit+ Exponent?);
Exponent = ('e' | 'E') ('+' | '-')? Digit+;
Digit = /[0-9]/;
```

### 1.0.3 - String escape sequences
String literals can contain zero or more escape sequences, which are special character sequences that will be replaced with another character (or set of characters) after the string has been parsed.

#### Escape sequences
| Escape sequence | Name |
|:---------------:|------|
| `\a`            | Alert |
| `\b`            | Backspace |
| `\f`            | Form feed |
| `\n`            | New line |
| `\r`            | Carriage return |
| `\t`            | Horizontal tab |
| `\v`            | Vertical tab |
| `\'`            | Single quote |
| `\"`            | Double quote |
| `\\`            | Backslash |
| `\xxx`          | Octal escape sequence |
| `\uxxxx`        | 2-byte Unicode escape sequence |
| `\Uxxxxxxxx`    | 4-byte Unicode escape sequence |
| `\{x}`          | String interpolation |

Octal escape sequences may contain 1-3 octal digits specifying the ASCII value of a character. 2-byte and 4-byte Unicode escape sequences must contain exactly 4 and 8 hexadecimal digits, respectively, which represent a Unicode code point. String interpolation escape sequences insert the specified expression into the string at runtime.

### 1.0.4 - Comment blocks
Comment blocks are (potentially) multi-line comments which can be written anywhere within a module except within string literals. Comment blocks begin with the sequence `/*` and end with the sequence `*/`. Comment blocks must be terminated by the `*/` sequence; you cannot end it with a newline, a carriage return, `EOF`, or anything else.

### 1.0.5 - Symbols
Accele defines a fixed set of valid symbols which can be used in the language. Symbols are sequences of one or more (usually) non-alphanumeric characters with no whitespace or comments between characters.

#### Valid symbols
| Symbol | Name |
|:------:|------|
| `~`    | Tilde |
| `!`    | Exclamation point |
| `%`    | Percent |
| `^`    | Caret |
| `&`    | Ampersand |
| `*`    | Asterisk |
| `(`    | Left parenthesis |
| `)`    | Right parenthesis |
| `-`    | Minus |
| `=`    | Equals |
| `+`    | Plus |
| `[`    | Left bracket |
| `]`    | Right bracket |
| `{`    | Left brace |
| `}`    | Right brace |
| `\|`    | Pipe |
| `;`    | Semicolon |
| `:`    | Colon |
| `<`    | Less than |
| `>`    | Greater than |
| `.`    | Dot |
| `/`    | Slash |
| `?`    | Question mark |
| `==`   | Double equals |
| `===`  | Triple equals |
| `~=`   | Tilde equals |
| `!=`   | Exclamation point equals |
| `!==`  | Exclamation point double equals |
| `%=`   | Percent equals |
| `^=`   | Caret equals |
| `&=`   | Ampersand equals |
| `*=`   | Asterisk equals |
| `-=`   | Minus equals |
| `+=`   | Plus equals |
| `\|=`   | Pipe equals |
| `<=`   | Less than equals |
| `>=`   | Greater than equals |
| `/=`   | Slash equals |
| `&&`   | Double ampersand |
| `\|\|`   | Double pipe |
| `**`   | Double asterisk |
| `--`   | Double minus |
| `++`   | Double plus |
| `<<`   | Double less than |
| `>>`   | Double greater than |
| `..`   | Double dot |
| `...`  | Triple dot |
| `??`   | Double question mark |
| `?.`   | Question mark dot |
| `->`   | Minus arrow |
| `=>`   | Equals arrow |
| `<=>`  | Compare |
| `**=`  | Double asterisk equals |
| `<<=`  | Double less than equals |
| `>>=`  | Double greater than equals |

### 1.0.6 - Meta keywords
Meta keywords are special keywords used to give the compiler additional information about a statement, symbol, or module to use during compile time. All meta keywords begin with the at-sign (`@`) and contain the same characters as a standard identifier.

Some meta keywords require arguments. These must be compile-time constants placed within parentheses immediately proceeding the meta keyword. For example, the `@aspenable` and `@aspdisable` tags require one or more arguments specifying the protocols to enable or disable. Arguments must be separated by commas.

Example - The following code prevents the compiler from issuing warnings for making calls to the deprecated function `foo`:
```
@deprecated
fun foo() {
    print("Hello")
}

fun main() {
    @disablewarning("deprecation")
    foo()
}
```

#### Valid meta keywords
| Keyword | Valid Locations | Description |
|:-------:|-----------------|-------------|
| `@noreturn` | Functions | Indicates that the following function does not return to its caller. |
| `@stackalloc` | Local variables | Indicates that the following local variable should be allocated on the stack regardless of any other indicators. |
| `@srclock` | Modules | Indicates that the containing module is visible only to other modules within the same containing directory. |
| `@laxthrow` | Classes, structures | Indicates that the following class or struct, which must be a subtype of `Exception`, does not require a `throwing` context to be thrown from a function. |
| `@externalinit` | Class variables | Indicates that the following variable is initialized externally (i.e. in C/C++). A module containing this tag will not be able to be directly compiled; it can only be compiled to C/C++ source. |
| `@deprecated` | All symbols | Indicates that the following symbol is deprecated and should not be used. |
| `@enablewarning` | Anywhere | Allows the compiler to check for the specified protocols and issue warnings about the protocols within the specified symbol, statement, scope, or module. |
| `@disablewarning` | Anywhere | Prevents the compiler from checking for the specified protocols and issuing warnings about the protocols within the specified symbol, statement, scope, or module. |

#### Grammar
```
MetaKeyword = SimpleMetaKeyword | WarningMetaKeyword;
SimpleMetaKeyword = '@noreturn' | '@stackalloc' | '@srclock' | '@laxthrow' | '@externalinit' | '@deprecated';
WarningMetaKeyword = ('@enablewarning' | '@disablewarning') '(' StringLiteral (',' StringLiteral)* ')';
```

### 1.0.7 - Global scope
The global scope is the top-level scope which encompasses the entirety of a given module. All variables, functions, types, and namespaces not declared within another scope are considered global symbols. The global scope can contain zero or more of: variables, constants, aliases, functions, classes, structures, templates, enumerations, namespaces, import declarations, and certain meta declarations. Superfluous newline-equivalent tokens are ignored.

#### Grammar
```
GlobalScope = (NonWarningMetaGlobalContent | GlobalWarningMeta)*;
NonWarningMetaGlobalContent = GlobalVariable | GlobalConstant | GlobalAlias | GlobalFunction | GlobalClass | GlobalStruct | GlobalTemplate | GlobalEnum | GlobalNamespace | Import | SourceLockMeta;
GlobalVariable = GlobalVariableModifier* 'var' ID (':' TypeRef)? ('=' Expression)? NL;
GlobalConstant = GlobalConstantModifier* 'const' ID (':' TypeRef)? '=' Expression NL;
GlobalAlias = GlobalAliasModifier* 'alias' ID Generics? '=' TypeRef NL;
GlobalFunction = GlobalFunctionModifier* 'fun' ID Generics? '(' Parameters? ')' ('->' TypeRef)? (('=' Expression NL) | FunctionBlock)?;
GlobalClass = GlobalClassModifier* 'class' ID Generics? (':' TypeRef)? ('uses' TypeRef (',' TypeRef)*)? ClassBlock;
GlobalStruct = GlobalStructModifier* 'struct' ID Generics? (':' TypeRef)? ('uses' TypeRef (',' TypeRef)*)? ClassBlock;
GlobalTemplate = GlobalTemplateModifier* 'template' ID Generics? (':' TypeRef (',' TypeRef)*)? ClassBlock;
GlobalEnum = GlobalEnumModifier* 'enum' ID Generics? ('uses' TypeRef (',' TypeRef)*)? EnumBlock;
GlobalNamespace = GlobalNamespaceModifier* 'namespace' ID Generics? NamespaceBlock;
Import = FromImport | StandardImport;
FromImport = 'import' (ImportTarget | '{' ImportTarget (',' ImportTarget)* '}') 'from' (ImportID | StringLiteral);
StandardImport = 'import' (ImportID | StringLiteral) ('as' ID)?;
SourceLockMeta = '@srclock';
GlobalWarningMeta = ('@enablewarning' | '@disablewarning') '(' StringLiteral (',' StringLiteral)* ')' NonWarningMetaGlobalContent;

GlobalVariableModifier = 'internal' | 'unsafe' | 'atomic' | '@deprecated' | 'strong' | 'weak' | 'greedy';
GlobalConstantModifier = 'internal' | 'unsafe' | 'atomic' | '@deprecated' | 'strong' | 'weak' | 'greedy';
GlobalAliasModifier = 'internal' | '@deprecated';
GlobalFunctionModifier = 'internal' | 'unsafe' | 'throwing' | 'noexcept' | '@noreturn' | 'async' | 'extern' | '@deprecated';
GlobalClassModifier = 'internal' | 'final' | '@laxthrow' | '@deprecated';
GlobalStructModifier = 'internal' | '@laxthrow' | '@deprecated';
GlobalTemplateModifier = 'internal' | '@deprecated';
GlobalEnumModifier = 'internal' | '@deprecated';
GlobalNamespaceModifier = 'internal' | '@deprecated';
```

### 1.0.8 - Line termination
Statements in Accele may be terminated by either a newline character sequence or a semicolon. A newline character sequence is a carriage return, a newline, or a carriage return followed by a newline. Statements may also be considered to terminate if the token immediately proceeding them is a right bracket, a right brace, a right parenthesis, or a comma. Such tokens however are not true line termination tokens and must be part of other language structures to be considered valid line terminations.

### 1.1.1 - Source-lock fronting
All `@srclock` declarations should be made at the top of the module before any other statement. This ensures that the reader immediately knows this module is source-locked and is not intended to be used outside of its containing directory.

### 1.1.2 - Deprecation
Deprecated symbols, which are tagged using the meta keyword `@deprecated`, are indicated as such to tell the end user to avoid using the symbol because it may be removed in a future version.