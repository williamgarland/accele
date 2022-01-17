# Accele Error Codes
This document defines the error codes that may be encountered during compilation.

## `ACL0000` - Unknown error
Occurs when there is an unknown error. These kinds of errors are usually irrecoverable and should not occur during normal compilation.
If you see this error, it probably means there is a problem with the compiler itself.

### Synopsis
- Error Code: `ACL0000`
- Title: `Unknown error`
- Problem Type: Error

## `ACL0001` - Symbol not visible
Occurs when the symbol you're trying to access is not visible from the scope in which it is referenced.

### Synopsis
- Error Code: `ACL0001`
- Title: `Symbol not visible`
- Problem Type: Error

## `ACL0002` - Invalid modifier
Occurs when a node has an invalid modifier. Examples of this include declaring a variable as `extern`, declaring a function as `strong`, declaring a block as `final`, etc.

### Synopsis
- Error Code: `ACL0002`
- Title: `Invalid modifier`
- Problem Type: Error

## `ACL0003` - Static access via instance
Occurs when you try to access a static member through an instance of the type. Static members should be accessed directly through the type instead of through an instance.

### Synopsis
- Error Code: `ACL0003`
- Title: `Static access via instance`
- Problem Type: Warning

## `ACL0004` - Instance access via static
Occurs when you try to access an instance member in a static way. Instance members must be accessed through an instance of the type; they cannot be accessed directly through the type itself.

### Synopsis
- Error Code: `ACL0004`
- Title: `Instance access via static`
- Problem Type: Error

## `ACL0005` - Generics mismatch
Occurs when you provide an invalid type as an argument to a generic parameter. Ordinarily, generic parameters can accept any type, however ones that declare a super-type are restricted to instances of that type and any of its sub-types.

### Synopsis
- Error Code: `ACL0005`
- Title: `Generics mismatch`
- Problem Type: Error

## `ACL0006` - Too many generics
Occurs when you provide too many generic arguments to a symbol that only accepts a certain number of generic arguments. For example, if a symbol `S` declares two generic parameters `T` and `U`, writing `S<Int, Double, Int>` would be an error since it only accepts two generic arguments; not three.

### Synopsis
- Error Code: `ACL0006`
- Title: `Too many generics`
- Problem Type: Error

## `ACL0007` - Insufficient generics
Occurs when you provide too few generic arguments to a symbol that accepts more than the provided number of arguments. For example, if a symbol `S` declares two generic parameters `T` and `U`, writing `S<Int>` would be an error since it accepts two generic arguments; not one. There is one exception to this rule: generic arguments can be inferred for certain function calls if the compiler is able to guess the generic arguments based on the types of the actual arguments provided in the call. If the compiler can't guess the missing generics, this error will be shown.

### Synopsis
- Error Code: `ACL0007`
- Title: `Insufficient generics`
- Problem Type: Error

## `ACL0008` - Duplicate symbol
Occurs when you declare a symbol with an identifier that has already been used elsewhere in the scope (or in a parent scope in certain situations). For example, if you declare a class with the ID `T`, you cannot also declare a template with the ID `T` in the same scope. If you're in a function scope, the compiler will look at the current scope and all parent scopes above it (up to and including the top-most function scope). For example:

```
fun f() {
    var x = 10
    {
        const x = true
    }
}
```

In the above example, the variable `x` was redeclared as a constant in the child scope, which the compiler considers to be a duplicate symbol.

### Synopsis
- Error Code: `ACL0008`
- Title: `Duplicate symbol`
- Problem Type: Error

## `ACL0009` - Duplicate import
Occurs when the same module is imported more than once in the same module. The way in which a module is imported is unimportant; if a module is imported twice, even if the second import is a different kind of import, the second import is considered a duplicate. For example:

```
import b
import x from b
```

The above imports are duplicates because both imports have the same target module `b`.

### Synopsis
- Error Code: `ACL0009`
- Title: `Duplicate import`
- Problem Type: Error

## `ACL0010` - Duplicate import alias
Occurs when two imports have the same alias. This applies even to imports that do not explicitly declare an alias. For example:

```
import ..a as b
import x from b
import b
import "x/y/z/b.accele"
```

All of the above imports have the same alias `b` even though all are different import methods and they import different modules.

### Synopsis
- Error Code: `ACL0010`
- Title: `Duplicate import alias`
- Problem Type: Error

## `ACL0011` - Argument type mismatch
Occurs when an argument to a function call has the wrong type. For example, if a function `f` declares a parameter `x` which is an `Int`, calling `f("a string")` is an error because an argument of type `String` cannot be automatically cast to an `Int`.

### Synopsis
- Error Code: `ACL0011`
- Title: `Argument type mismatch`
- Problem Type: Error

## `ACL0012` - Too many arguments
Occurs when too many arguments are provided to a function call. For example, if a function `f` has one parameter `x` which is an `Int`, calling `f(1, 2)` is an error because `f` only accepts one argument.

### Synopsis
- Error Code: `ACL0012`
- Title: `Too many arguments`
- Problem Type: Error

## `ACL0013` - Insufficient arguments
Occurs when too few arguments are provided to a function call. For example, if a function `f` has two parameters `x` and `y`, calling `f(1)` is an error because `f` expects two arguments; not one.

### Synopsis
- Error Code: `ACL0013`
- Title: `Insufficient arguments`
- Problem Type: Error

## `ACL0014` - Static self
Occurs when the `self` keyword is used in a static context. The `self` keyword refers to the current instance, so using it outside of an instance function, constructor, or destructor is an error.

### Synopsis
- Error Code: `ACL0014`
- Title: `Static self`
- Problem Type: Error

## `ACL0015` - Static super
Occurs when the `super` keyword is used in a static context. Like `self`, `super` refers to the current instance (but cast to one of its parent types), so using it outside of an instance function, constructor, or destructor is an error.

### Synopsis
- Error Code: `ACL0015`
- Title: `Static super`
- Problem Type: Error

## `ACL0016` - Invalid comment block end
Occurs when a multi-line comment block is not terminated correctly. You cannot have a trailing multi-line comment block at the end of the module; it must be terminated with the character sequence `*/`.

### Synopsis
- Error Code: `ACL0016`
- Title: `Invalid comment block end`
- Problem Type: Error

## `ACL0017` - Invalid float literal
Occurs when a floating-point literal is not written correctly. Refer to the grammar on how floating-point literals should be written.

### Synopsis
- Error Code: `ACL0017`
- Title: `Invalid float literal`
- Problem Type: Error

## `ACL0018` - Invalid hex literal
Occurs when a hexadecimal integer literal is not written correctly. Refer to the grammar on how hexadecimal integer literals should be written.

### Synopsis
- Error Code: `ACL0018`
- Title: `Invalid hex literal`
- Problem Type: Error

## `ACL0019` - Invalid octal literal
Occurs when an octal integer literal is not written correctly. Refer to the grammar on how octal integer literals should be written.

### Synopsis
- Error Code: `ACL0019`
- Title: `Invalid octal literal`
- Problem Type: Error

## `ACL0020` - Invalid binary literal
Occurs when a binary integer literal is not written correctly. Refer to the grammar on how binary integer literals should be written.

### Synopsis
- Error Code: `ACL0020`
- Title: `Invalid binary literal`
- Problem Type: Error

## `ACL0021` - Invalid lexical symbol
Occurs when a lexical symbol (i.e. an operator, a punctuation mark, etc.) contains invalid characters.

### Synopsis
- Error Code: `ACL0021`
- Title: `Invalid lexical symbol`
- Problem Type: Error

## `ACL0022` - Invalid Unicode escape sequence
Occurs when a Unicode string escape sequence is invalid, such as containing insufficient digits.

### Synopsis
- Error Code: `ACL0022`
- Title: `Invalid Unicode escape sequence`
- Problem Type: Error

## `ACL0023` - Invalid interpolation escape sequence
Occurs when a string interpolation escape sequence is invalid. Refer to the grammar on how interpolation escape sequences should be written.

### Synopsis
- Error Code: `ACL0023`
- Title: `Invalid interpolation escape sequence`
- Problem Type: Error

## `ACL0024` - Invalid escape sequence
Occurs when a string escape sequence is invalid. Refer to the grammar on which escape sequences are available in Accele.

### Synopsis
- Error Code: `ACL0024`
- Title: `Invalid escape sequence`
- Problem Type: Error

## `ACL0025` - Invalid string literal end
Occurs when a string literal is not terminated. You cannot have a trailing string literal at the end of the module; it must be terminated with the same delimiter character with which it began.

### Synopsis
- Error Code: `ACL0025`
- Title: `Invalid string literal end`
- Problem Type: Error

## `ACL0026` - Invalid input
Occurs when there is an invalid character (not within a string literal or comment) in the module. For example, the `#` character is not supported in Accele files, so unless it is within a string literal or a comment, it cannot be written in the file.

### Synopsis
- Error Code: `ACL0026`
- Title: `Invalid input`
- Problem Type: Error

## `ACL0027` - Invalid tag
Occurs when an invalid tag is declared. Refer to the grammar on which tags are valid.

### Synopsis
- Error Code: `ACL0027`
- Title: `Invalid tag`
- Problem Type: Error

## `ACL0028` - Invalid token
Occurs when the compiler was expecting to find a particular kind of token, but instead received a different one.

### Synopsis
- Error Code: `ACL0028`
- Title: `Invalid token`
- Problem Type: Error

## `ACL0029` - Duplicate variable block
Occurs when a variable block (i.e. `get`, `set`, and `init`) was declared more than once for the same variable or constant.

### Synopsis
- Error Code: `ACL0029`
- Title: `Duplicate variable block`
- Problem Type: Error

## `ACL0030` - Non-static template variable
Occurs when a variable (or constant) inside a template is not declared as `static`. All variables within templates must be static.

### Synopsis
- Error Code: `ACL0030`
- Title: `Non-static template variable`
- Problem Type: Error

## `ACL0031` - Non-fronted source lock tag
Occurs when the `@srclock` tag is not declared at the top of the module. All source lock declarations should be placed at the top of the module.

### Synopsis
- Error Code: `ACL0031`
- Title: `Non-fronted source lock tag`
- Problem Type: Warning

## `ACL0032` - Duplicate default switch case
Occurs when the default case in a switch block was declared more than once. A switch block may only have one default case.

### Synopsis
- Error Code: `ACL0032`
- Title: `Duplicate default switch case`
- Problem Type: Error

## `ACL0033` - Non-final variadic parameter
Occurs when a parameter is declared variadic, but it is not the last parameter in the list. There can only be one variadic parameter per function and it must be the last parameter in the list.

### Synopsis
- Error Code: `ACL0033`
- Title: `Non-final variadic parameter`
- Problem Type: Error

## `ACL0034` - Invalid return statement
Occurs when the value returned in a return statement is incompatible with the return type of the containing function. For example, if the function is supposed to return an `Int`, you can't write `return "some string"`.

### Synopsis
- Error Code: `ACL0034`
- Title: `Invalid return statement`
- Problem Type: Error

## `ACL0035` - Invalid throw statement
Occurs when the value returned in a throw statement is not an exception type. All values thrown using the `throw` keyword must be instances or sub-types of the `Exception` type.

### Synopsis
- Error Code: `ACL0035`
- Title: `Invalid throw statement`
- Problem Type: Error

## `ACL0036` - Invalid function caller
Occurs when the caller of a function call expression is not a function or does not have a function type.

### Synopsis
- Error Code: `ACL0036`
- Title: `Invalid function caller`
- Problem Type: Error

## `ACL0037` - Undefined symbol
Occurs when a symbol is referenced before it is defined.

### Synopsis
- Error Code: `ACL0037`
- Title: `Undefined symbol`
- Problem Type: Error

## `ACL0038` - Invalid symbol for expression
Occurs when an invalid symbol is the target of an expression. Namespaces, module aliases, and types cannot be the target of an expression (except for casting expressions).

### Synopsis
- Error Code: `ACL0038`
- Title: `Invalid symbol for expression`
- Problem Type: Error

## `ACL0039` - Template constructor
Occurs when you attempt to construct an instance of a template type. Templates cannot have direct instances; they can only be parent types to classes, structures, enumerations, and other templates.

### Synopsis
- Error Code: `ACL0039`
- Title: `Template constructor`
- Problem Type: Error

## `ACL0040` - Unresolved symbol
Occurs when a reference to a symbol is unresolved.

### Synopsis
- Error Code: `ACL0040`
- Title: `Unresolved symbol`
- Problem Type: Error

## `ACL0041` - Unresolved import
Occurs when an import does not point to an actual module.

### Synopsis
- Error Code: `ACL0041`
- Title: `Unresolved import`
- Problem Type: Error