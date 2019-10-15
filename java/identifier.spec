# An identifier is an unlimited-length sequence of Java letters and Java digits, the
# first of which must be a Java letter
#
# TODO: So far we dont support unicode which are not major goal right now.

rule JavaChar : ONEOF(CHAR, '_' , '$')
rule IDENTIFIER : JavaChar + ZEROORMORE(CHAR, DIGIT)
