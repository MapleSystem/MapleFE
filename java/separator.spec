# This file defines the separator. The separators are defined using a STRUCT.
# Each separator in this STRUCT is a set of 2 elements.
# STRUCT SEPARATOR : ( ("(", LeftParenthesis),
#                      (")", RightParenthesis),
# The first element is the literal name of separator, it needs to be a string
# The second is the ID of the separator. Please check autogen/seps_supported.spec
# to see the supported separator ID.

STRUCT SEPARATOR : ((" ", WhiteSpace),
                    ("(", LeftParenthesis),
                    (")", RightParenthesis),
                    ("{", LeftBrace),
                    ("}", RightBrace),
                    ("[", LeftBracket),
                    ("]", RightBracket),
                    (";", SemiColon),
                    (",", Comma),
                    (".", Period),
                    ("...", Omission),
                    ("@", At),
                    ("::", Of))
