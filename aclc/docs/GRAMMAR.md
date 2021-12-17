This document describes the grammar of Accele.

# Expressions
```
Expression = AssignmentExpression;
```

## Assignment Expressions
```
AssignmentExpression = L2Expression (AssignmentOperator AssignmentExpression)?;
AssignmentOperator = '=' | '+=' | '-=' | '*=' | '/=' | '%=' | '**=' | '&=' | '|=' | '^=' | '<<=' | '>>=' | '~=';
L2Expression = LambdaExpression | TernaryExpression;
```

## Lambda Expressions
```
LambdaHeader '=>' LambdaBody;
LambdaHeader = 'async'? LambdaParameters;
LambdaParameters = Parameter | ('(' Parameter (',' Parameter)* ')');
LambdaBody = Expression | FunctionBlock;
```

## Ternary Expressions
```
TernaryExpression = LogicalOrExpression ('?' Expression ':' Expression)?;
```

## Logical OR Expressions
```
LogicalOrExpression = LogicalAndExpression (LogicalOrOperator LogicalAndExpression)*;
LogicalOrOperator = '||' | 'or';
```

## Logical AND Expressions
```
LogicalAndExpression = BitwiseOrExpression (LogicalAndOperator BitwiseOrExpression)*;
LogicalAndOperator = '&&' | 'and';
```

## Bitwise OR Expressions
```
BitwiseOrExpression = BitwiseXorExpression ('|' BitwiseXorExpression)*;
```

## Bitwise XOR Expressions
```
BitwiseXorExpression = BitwiseAndExpression ('^' BitwiseAndExpression)*;
```

## Bitwise AND Expressions
```
BitwiseAndExpression = EqualityExpression ('&' EqualityExpression)*;
```

## Equality Expressions
```
EqualityExpression = RelationalExpression (EqualityOperator RelationalExpression)*;
EqualityOperator = '==' | '!=' | '===' | '!==';
```

## Relational Expressions
```
RelationalExpression = NilCoalescingExpression (RelationalOperator NilCoalescingExpression)*;
RelationalOperator = '<' | '>' | '<=' | '>=' | '<=>';
```

## Nil-Coalescing Expressions
```
NilCoalescingExpression = CastingExpression ('??' CastingExpression)*;
```

## Casting Expressions
```
CastingExpression = RangeExpression (CastingOperator Type)*;
CastingOperator = 'is' | 'as' | 'as?' | 'as!';
```

## Range Expressions
```
RangeExpression = BitshiftExpression (RangeOperator BitshiftExpression)*;
RangeOperator = '..' | '...';
```

## Bitshift Expressions
```
BitshiftExpression = AdditiveExpression (BitshiftOperator AdditiveExpression)*;
BitshiftOperator = '<<' | '>>';
```

## Additive Expressions
```
AdditiveExpression = MultiplicativeExpression (AdditiveOperator MultiplicativeExpression)*;
AdditiveOperator = '+' | '-';
```

## Multiplicative Expressions
```
MultiplicativeExpression = ExponentialExpression (MultiplicativeOperator ExponentialExpression)*;
MultiplicativeOperator = '*' | '/' | '%';
```

## Exponential Expressions
```
ExponentialExpression = PrefixExpression ('**' PrefixExpression)*;
```

## Prefix Expressions
```
PrefixExpression = PrefixOperator* PostfixExpression;
PrefixOperator = '+' | '-' | '++' | '--' | '~' | '!' | 'not' | '*' | '&' | 'release' | 'try?' | 'try!' | 'await';
```

## Postfix Expressions
```
PostfixExpression = AccessCallExpression PostfixOperator*;
PostfixOperator = '++' | '--' | '!';
```

## Access and Call Expressions
```
AccessCallExpression = PrimaryExpression (AccessExpressionEnd | CallExpressionEnd)*;
AccessExpressionEnd = AccessOperator PrimaryExpression;
AccessOperator = '.' | '?.';
CallExpressionEnd = FunctionCallExpressionEnd | SubscriptExpressionEnd;
FunctionCallExpressionEnd = '(' Arguments? ')';
Arguments = Expression (',' Expression)*;
SubscriptExpressionEnd = '[' Expression ']';
```

## Primary Expressions
```
PrimaryExpression = LiteralExpression | BaseID | MapExpression | ArrayExpression | TupleExpression | ('(' Expression ')');
LiteralExpression = IntegerLiteral | HexadecimalLiteral | OctalLiteral | BinaryLiteral | FloatLiteral | StringLiteral | BooleanLiteral | NilLiteral | 'self' | 'super';
MapExpression = '{' MapEntries? '}';
MapEntries = MapEntry (',' MapEntry)*;
MapEntry = Expression ':' Expression;
ArrayExpression = '[' (Expression (',' Expression)*)? ']';
TupleExpression = '(' Expression (',' Expression)+ ')';
BaseID = ('global' '.')? ID Generics?;
```

# Identifiers
```
ID = IdentStart IdentPart*;
IdentStart = /[a-zA-Z_$]/;
IdentPart = IdentStart | /[0-9]/;
```

# Integer Literals
```
IntegerLiteral = Digit+;
Digit = /[0-9]/;
```

# Hexadecimal Literals
```
HexadecimalLiteral = '0' ('x' | 'X') HexDigit+;
HexDigit = /[a-fA-F0-9]/;
```

# Octal Literals
```
OctalLiteral = '0' ('o' | 'O') OctalDigit+;
OctalDigit = /[0-7]/;
```

# Binary Literals
```
BinaryLiteral = '0' ('b' | 'B') BinaryDigit+;
BinaryDigit = '0' | '1';
```

# Floating-Point Literals
```
FloatLiteral = (Digit+ '.' Digit* Exponent?) | ('.'? Digit+ Exponent?);

Exponent = ('e' | 'E') ('+' | '-')? Digit+;
```

# String Literals
```
StringLiteral = SingleQuotedStringLiteral | DoubleQuotedStringLiteral;

SingleQuotedStringLiteral = "'" SingleQuotedStringContent* "'";
SingleQuotedStringContent = EscapeSequence | /[^']/;

DoubleQuotedStringLiteral = '"' DoubleQuotedStringContent* '"';
DoubleQuotedStringContent = EscapeSequence | /[^"]/;

EscapeSequence = '\\' (SimpleEscapeSequence | ShortUnicodeEscapeSequence | LongUnicodeEscapeSequence | OctalEscapeSequence | InterpolationEscapeSequence);

SimpleEscapeSequence = /[abfnrtv'"\\]/;
ShortUnicodeEscapeSequence = 'u' /[a-fA-F0-9]{4}/;
ShortUnicodeEscapeSequence = 'U' /[a-fA-F0-9]{8}/;
OctalEscapeSequence = /[0-7]{1,3}/;

InterpolationEscapeSequence = '{' Expression '}';
```

# Boolean Literals
```
BooleanLiteral = 'true' | 'false';
```

# Nil Literals
```
NilLiteral = 'nil';
```