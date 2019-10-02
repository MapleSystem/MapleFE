This file defines the syntax of AutoGen which is common to all the *.spec.
Each *.spec has its own special definitions of how to write correct syntax rules
or structures.

________
Comments
________

Lines starting with '#' is considered as a comment.

_______
Reserve
_______

AutoGen reserves some keyword, and here they are.

 *) rule
    Every statement in .spec file starts with 'rule'. The rule can cross multiple
    lines until it hits the next 'rule'.

 *) ONEOF
    An Operation. Means as it names.

 *) ZEORORMORE
    An Operation. zero or more

 *) ZEORORONE
    An Operation. zero or one

 *) STRUCT
    This is a specific structure for each *.spec. It defines special syntax in each
    different catogery. It has the format of:
      STRUCT(struct-name) = ( (...), (...), ...)

    A very common example is STRUCT, which defines the syntax 'keyword'
    each category includes.  e.g. in operator.spec,
      STRUCT Keyword  : (("=", Assign), ("-", Minus), ...)

    each *.spec has different meaning of its STRUCT(...), so they will handled
    by its own XxxGen. This is the reason you see a 'virtual' function
    ReadStructure(...) in BaseGen, and implemented by individual XxxGen.

 *) ( )
    A delimiter for Operations. It defines a set. Remember, if you want to mean
    the char in the target source language, please use single quote to embrace it,
    e.g. '('

 *) IDENTIFIER, SEPARATOR, OPERATOR, LITERAL
    A set. This matches the corresponding part in the lexer/parser

 *) CHAR
    A set. Only contains the latin ASCII characters, meaning a-z, A-Z

 *) DIGIT
    A set. 0 - 9

_____________________
Basic Syntax
_____________________

 *) Only one rule element on the right hand side of each rule. This rule element
    can contain multiple sub elements.

 *) One rule can be broken into multiple lines, e.g.
      rule SEPARATOR : ANYOF ( "{",
                              "}" )
    However, an element cannot cross multiple lines.

 *) "+" is used as concatentation.

 *) ',' is used as separator between elements in a set.

 *) Each line start with (1) either 'rule'  (2) or continuing with previous line

 *) Each operation is followed by a pair of parenthesis, ( and ), which embrace
    a set of elements.

 *) Literal.
    There are two types of literals supported. (1) literal character, embraced by
    single quote, e.g. 'c'; and (2) literal string embraced by quote, e.g. "abc".

    Empty string is allowed, i.e. "" is legal representing empty string.
