#######################################################################
# There are over 10 kinds of type conversion in Java. This file       #
# defines the rules of type conversion in different categories.       #
#                                                                     #
# There are many details putting many flavors in the semantics.       #
# They can be implemented in java/src/*.cpp. Such as when convert     #
# 'float' to 'Float', it diverges when the value is 'NaN'.            #
# Situations like this will be handled in specified functions.        #
#######################################################################


#######################################################################
#            Identity Conversion                                      #
#######################################################################


#######################################################################
#            Widening Primitive Conversion                            #
#######################################################################


#######################################################################
#            Narrowing Primitive Conversion                           #
#######################################################################


#######################################################################
#            Widening Reference Conversion                            #
#######################################################################


#######################################################################
#            Narrowing Reference Conversion                           #
#######################################################################


#######################################################################
#            Boxing Conversion                                        #
#######################################################################


#######################################################################
#            Unboxing Conversion                                      #
#######################################################################


#######################################################################
#            Unchecked Conversion                                     #
#######################################################################


#######################################################################
#            Capture Conversion                                       #
#######################################################################


#######################################################################
#            String Conversion                                        #
#######################################################################


#######################################################################
#            Forbidden Conversion                                     #
#######################################################################


#######################################################################
#            Value Set Conversion                                     #
#######################################################################


#######################################################################
#  Boxing/Unboxing conversion is conversion between primitive types   #
#  and their corresponding Java reference types.                      #
#  This could also happen in other similar lanaguages.                #
#                                                                     #
#  1. Since reference types are not part of Autogen, they are not     #
#     handled in the supported_types.spec.                            #
#  2. Reference types here will be only recognized through keyword,   #
#     The generated files will handle the conversion.                 #
#                                                                     #
# TODO: There is one thing I will figure out later, i.e., see if we   #
#       need generate the 'new' operation in the Maple IR             #
#######################################################################

# The syntax of Boxing is a duplex <PrimType, "RefType">
#   PrimType: Use the types supported in autogen/supported_types.spec
#   RefType: Use the keyword of reference types
#

STRUCT Boxing : ONEOF((Boolean, "Boolean"),
                      (Byte, "Byte"),
                      (Short, "Short"),
                      (Char, "Character"),
                      (Int, "Integer"),
                      (Long, "Long"),
                      (Float, "Float"),
                      (Double, "Double"))

STRUCT UnBoxing : ONEOF(("Boolean", Boolean),
                        ("Byte", Byte),
                        ("Short", Short),
                        ("Char", Char),
                        ("Int", Int),
                        ("Long", Long),
                        ("Float", Float),
                        ("Double", Double))

