# An identifier is an unlimited-length sequence of Java letters and Java digits, the
# first of which must be a Java letter
#
# https://docs.oracle.com/javase/specs/jls/se12/html/jls-3.html#jls-Identifier
#
# TODO: So far we dont support unicode which are not major goal right now.

rule JavaLetter: ONEOF(CHAR, '_' , '$')
rule JavaLetterOrDigit: ONEOF(JavaLetter, DIGIT)
rule IdentifierChars: JavaLetter + ZEROORMORE(JavaLetterOrDigit)
rule Identifier: IdentifierChars #but not a Keyword or BooleanLiteral or NullLiteral
