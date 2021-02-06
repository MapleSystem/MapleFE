This file defines the syntax of AutoGen which is common to all the *.spec.
Each *.spec has its own special definitions of how to write correct syntax rules
or structures.

____________________________________

Section 1. Comments
____________________________________

Lines starting with '#' is considered as a comment.

_____________________________________

Section 2. Reserved or system supported items
_____________________________________

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

 *) Identifier, Separator, Operator, Literal
    Each is a set. This matches the corresponding part in the lexer/parser

 *) CHAR
    A set. Only contains the latin ASCII characters, meaning a-z, A-Z

 *) DIGIT
    A set. 0 - 9

 *) ASCII
    Ascii set.

 *) ESCAPE
    Escape set.

 *) System Supported data types
    Boolean, Byte, Short, Int, Long, Char, Float, Double, Null
    These can be used in the <attr.type> of a rule.

 *) System Supported functions

    The rules has profound ways to describe its semantics. This requires a set of
    functions in language parser which check the validity of rule elements, or
    generate the expression/statemets in the final compiler IR.

    Autogen generate the code to do these validity checking and IR generation, by
    understanding the attributes of each rule.

    The detailed list of supported functions is in Section Appendix in the end of this
    document.
_____________________

Section 3. Must Have Rules/STRUCTs
_____________________

  From the aspect of language syntax, for every different programming languages,  
  the common components include separator, operator, literal, type, identifier,
  expression, and statement. So, to define your own set of .spec, these rules are
  must-have so that the parser/lexer/autogen can find the right place to start
  their work.

  *) rule Expression
  *) rule Statement
  *) rule Identifier
  *) rule Literal
    *) IntergerLiteral
    *) FPLiteral
    *) BooleanLiteral
    *) CharLiteral
    *) StringLiteral
    *) NullLiteral
    *) ThisLiteral
    *) SuperLiteral
  *) STRUCT Separator
  *) STRUCT Operator
  *) STRUCT Keyword
_____________________

Section 4. Rule Syntax
_____________________

 *) Only one rule element on the right hand side of each rule. This rule element
    can contain multiple sub elements.

 *) One rule can be broken into multiple lines, e.g.
      rule SEPARATOR : ONEOF ( "{",
                              "}" )
    An element can be multiple lines.

 *) "+" is used as concatentation.

 *) ',' is used as separator between elements in a set.

 *) Each line start with (1) either 'rule'  (2) or continuing with previous line

 *) Each operation is followed by a pair of parenthesis, ( and ), which embrace
    a set of elements.

 *) Literal.
    There are two types of literals supported. (1) literal character, embraced by
    single quote, e.g. 'c'; and (2) literal string embraced by quote, e.g. "abc".

    Empty string is allowed, i.e. "" is legal representing empty string.

_____________________

Section 5. Rule Attributes
_____________________

 The rule syntax above describes the components of a rule, but there are many more
 semantic related information need be added. So we define a set of attributes that
 can be applied to a rule.

 *) attr.type
 *) attr.validity[.%n,%m] optional for specific elements #n, #m in ONEOF
 *) attr.action[.%n,%m]   optional for specific elements #n, #m in ONEOF
 *) attr.property : XXX

 Note : all indexes start from 1

 The basic synatx of attributes are similar as rule's, with the common format:
    attr.type : XXX
 XXX is the value of attribute. It must be a system recognized word.
 An example is shown below.

 rule Rule1: ONEOF(Rule2 + '+' + Rule3,
                   Rule2 + Rule4 + Rule3,
                   Rule4 + Rule5,
                   Rule6)
      attr.type     : Integer
      attr.validity.%1,%2 : IsUnsigned(%3)
      attr.validity.%2    : IsUnsigned(%2)
      attr.validity       : IsUnsigned(%1)
      attr.action.%1,%2   : GenerateBinaryExpr(%1, %2, %3)
      attr.action         : GenerateUnaryExpr(%1), BuildXXXExpr(%1)
      attr.property,%1,%2 : Single, Top

 1. attr.xxx           must follow the rule which it attaches to.
 2. attr.type          tells the data type of the generated IR component of this rule.
 3. attr.property      tells the properties this rule has. It could be a property having
                       impact on the parsing result, or a property affect parsing efficiency.
                       There will be a set of properties to support. Right now we support
                       two, ie. attr.property : Single, Top

                       [Single]
                       This is applied to a ONEOF rule, telling the parser the first
                       matching children rule is good. Don't need try all the children since
                       there will be ONLY one single children is valid for a specific
                       set of tokens. So it stops at the first child matching.

                       [Top]
                       This is applied to a Top rule, which is a top rule parser starts to
                       traverse. Usually there will be a few top rules in a language. For
                       example, in Java, package, import, class declaration and interface
                       declaration are all top rules. If you forget to add this property,
                       parser won't handle the code of this rule.

                       [Longest]
                       This is mostly applied to Concatenate rule, where we match the longest
                       possible input. This happens mostly in Lexer rules where we need
                       find a literal. Most rules have a separator to define the border of
                       a token, for example, char literal has ' to include it. string literal
                       has " to include. However, for hex literal, 0x1001, it could be
                       treated as 0x10 and 01, if there is no separator to include.
                       But in reality, most the language spec doesn't define the separator
                       for such a literal. All languages assume literals should be matched
                       to the longest possible.
                       The make the lexer work well, we request to put [Longest] property
                       if the rule does expect the longest match. Please see examples
                       in literal.spec of Java.

 4. attr.validity[.%n] tells the semantic limitation of each component, with optional
                       specific element number.  The corresponding functions will be
                       called by language parser.
 5. attr.action[.%n]   tells the action (or functions) to be invoked by language parser
                       when this rule is hit with optional specific element number.
                       Usually this tells the way to generate IR on target compiler.
 6. The value must be system recognized value, such as a type value.
    Or, it can be a system supported functions.
    Please refer to Section 2 <Reserved or system supported items> and Appendix
    for <System Supported Functions>
 7. Each term in rhs of the rule is given a number, starting from 1. It can
    be noted as %1, %2 and etc. In the above example, %1 stands for "Rule2 + '+' + Rule3".

_________________________________________________

Section 6. Separator
_________________________________________________

 *) WhiteSpace is treated as a separator.
    We recommend list WhiteSpace as your first separator since this is the top 1
    separators we hit in all kinds of languages. Putting it at the first place can
    speed up the parsing.

__________________________________________________

Appendix 1.  System Supported Functions
__________________________________________________

  The list of autogen supported functions are as below.
  1. IsUnsigned(%x)
  2. GenerateBinaryExpr(%x, %y)
  3. GenerateUnary(%x)
__________________________________________________

Appendix 2.  Tips for Spec Writers
__________________________________________________

Tip 1. When you write a rule production, and it's ending with ZEROORONE(...) + YYY or
       ZEROORMORE(...) + YYY, please pay attention. The matching phase walks through the
       rule elements and tries to match as much as it can for each element. This means
       if ZEROORONE(...) can eat up all the text until the end if it can also match the
       data for YYY, and YYY will never get a chance to get matched. This results in a
       failure of matching.

       Luckily, the matching algorithm will handle this by a second try which will match
       YYY at first and then match the ZEROORONE(...). However it has a limitation for now
       which is it only handles one single ZEROORONE(...) followed by another element.

       So please keep in mind, if you do have such a need please make sure there is only
       one single ZEROORONE(...) in such cases.
