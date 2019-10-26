# This file defines the separator. The separators are defined using a STRUCT.
# Each separator in this STRUCT is a set of 2 elements.
# STRUCT Separator : ( ("(", LeftParenthesis),
#                      (")", RightParenthesis),
# The first element is the literal name of separator, it needs to be a string
# The second is the ID of the separator. Please check shared/include/supported_separators.def
# to see the supported separator ID.

STRUCT Separator : ((" ", Whitespace),
                    ("(", Lparen),
                    (")", Rparen),
                    ("{", Lbrace),
                    ("}", Rbrace),
                    ("[", Lbrack),
                    ("]", Rbrack),
                    (";", Semicolon),
                    (",", Comma),
                    (".", Dot),
                    ("...", Dotdotdot),
                    (":", Colon),
                    ("::", Of),
                    ("@", At),
                    ("#", Pound))
