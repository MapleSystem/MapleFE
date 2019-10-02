# This file defines reserved rule elements.
#
# If a rule is without content, then it is like reserved keyword which
# autogen understand.
#
# TODO: if the range expression is implemented, then I can use a-b, A-B
#

# CHAR refers to the 26 letters in autogen
rule CHAR  : ONEOF('a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z', 'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z')

# DIGIT refers to the 10 digits
rule DIGIT : ONEOF('0', '1', '2', '3', '4', '5', '6', '7', '8', '9')
