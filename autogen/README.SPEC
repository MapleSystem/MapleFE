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

 *) IDENTIFIER, SEPARATOR, OPERATOR, LITERAL
    Each is a set. This matches the corresponding part in the lexer/parser

 *) CHAR
    A set. Only contains the latin ASCII characters, meaning a-z, A-Z

 *) DIGIT
    A set. 0 - 9

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
  *) rule IDENTIFIER
  *) rule Literal
  *) STRUCT SEPARATOR
  *) STRUCT OPERATOR
_____________________

Section 4. Rule Syntax
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

_____________________

Section 5. Rule Attributes
_____________________

 The rule syntax above describes the components of a rule, but there are many more
 semantic related information need be added. So we define a set of attributes that
 can be applied to a rule.

 *) attr.type
 *) attr.validity
 *) attr.action

 The basic synatx of attributes are similar as rule's, with the common format:
    attr.type : XXX
 XXX is the value of attribute. It must be a system recognized word.
 An example is shown below.

 rule IntegerLiteral: ONEOF(DecimalIntegerLiteral,
                            HexIntegerLiteral,
                            OctalIntegerLiteral,
                            BinaryIntegerLiteral)
      attr.type     : Integer
      attr.validity : IsUnsigned(%1); IsUnsigned(%2)
      attr.action   : GenerateBinaryExpr(%1, %2, %3);
                      GenerateUnaryExpr(%4)

 1. attr.xxx must follow the rule which it attaches to.
 2. attr.type     tells the data type of the generated IR component of this rule.
 3. attr.validity tells the semantic limitation of each component. The corresponding
                  functions will be called by language parser.
 4. attr.action   tells the action (or functions) to be invoked by language parser
                  when this rule is hit. Usually this tells the way to generate
                  IR on target compiler.
 5. The value must be system recognized value, such as a type value.
    Or, it can be a system supported functions.
    Please refer to Section 2 <Reserved or system supported items> and Appendix
    for <System Supported Functions>
 6. Each Non-Terminate in RHS of the rule is given a number, starting from 1. It can
    be noted as %1, %2 and etc. In the above example, %1 stands for DecimalIntegerLiteral.

_________________________________________________

Section 6. Separator
_________________________________________________

 *) WhiteSpace is treated as a separator.
    We recommend list WhiteSpace as your first separator since this is the top 1
    separators we hit in all kinds of languages.

__________________________________________________

Appendix.  System Supported Functions
__________________________________________________

  The list of autogen supported functions are as below.
  1. IsUnsigned(%x)
  2. GenerateBinaryExpr(%x, %y)
  3. GenerateUnary(%x)
