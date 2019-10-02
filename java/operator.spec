##########################################################################
# The syntax of operator.spec contains two parts.
#  1. The Keyword STRUCT defining the <keyword,opr> of all operations.
#        keyword : the text defined in the language
#        opr     : the standard Autogen defined operator.
#                  See autogen/include/supported_operators.def
#  2. The rule part, defining the language restrictions of each operator.
##########################################################################

STRUCT OPERATOR : ONEOF(
                    # Arithmetic
                    ("+",    Add),
                    ("-",    Sub),
                    ("*",    Mul),
                    ("/",    Div),
                    ("%",    Mod),
                    ("++",   Inc),
                    ("--",   Dec),
                    # Relation
                    ("==",   EQ),
                    ("!=",   NE),
                    (">",    GT),
                    ("<",    LT),
                    (">=",   GE),
                    ("<=",   LE),
                    # Bitwise
                    ("&",    Band),
                    ("|",    Bor),
                    ("^",    Bxor),
                    ("~",    Bcomp),
                    # Shift
                    ("<<",   Shl),
                    (">>",   Shr),
                    (">>>",  Zext),
                    # Logical
                    ("&&",   Land),
                    ("||",   Lor),
                    ("!",    Not),
                    # Assign
                    ("=",    Assign),
                    ("+=",   AddAssign),
                    ("-=",   SubAssign),
                    ("*=",   MulAssign),
                    ("/=",   DivAssign),
                    ("%=",   ModAssign),
                    ("<<=",  ShlAssign),
                    (">>=",  ShrAssign),
                    ("&=",   BandAssign),
                    ("|=",   BorAssign),
                    ("^=",   BxorAssign),
                    (">>>=", ZextAssign),
                    #
                    ("->",   Arrow),
                    (":",    Select),
                    ("?",    Cond))
