# A literal is the source code representation of a value of a primitive type, the
# String type, or the null type.
#
# NOTE: Make sure there is a 'rule Literal'. This is the official rule recognized
#       by autogen.

#########################################################################
##                          Integer                                    ##
#########################################################################

### Decimal rules

rule NonZeroDigit   : ONEOF('1', '2', '3', '4', '5', '6', '7', '8', '9')
rule Underscores    : '_' + ZEROORMORE('_')
rule Digit          : ONEOF('0', NonZeroDigit)
rule DigitOrUnderscore : ONEOF(Digit, '_')
rule DigitsAndUnderscores : DigitOrUnderscore + ZEROORMORE(DigitOrUnderscore)
rule Digits         : ONEOF(Digit, Digit + ZEROORONE(DigitsAndUnderscores) + Digit)

rule DecimalNumeral : ONEOF('0', NonZeroDigit + ZEROORONE(Digits),
                            NonZeroDigit + Underscores + Digits)

### Hexadecimal rules

rule HexDigit   : ONEOF('0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
                        'a', 'b', 'c', 'd', 'e', 'f',
                        'A', 'B', 'C', 'D', 'E', 'F')
rule HexDigitOrUnderscore : ONEOF(HexDigit, '_')
rule HexDigitsAndUnderscores:HexDigitOrUnderscore + ZEROORMORE(HexDigitOrUnderscore)
rule HexDigits  : ONEOF(HexDigit,
                        HexDigit + ZEROORONE(HexDigitsAndUnderscores) + HexDigit)
rule HexNumeral : ONEOF("0x" + HexDigits, "0X" + HexDigits)

### Octal rules

rule OctalDigit   : ONEOF('0', '1', '2', '3', '4', '5', '6', '7')
rule OctalDigitOrUnderscore : ONEOF(OctalDigit, '_')
rule OctalDigitsAndUnderscores:OctalDigitOrUnderscore + ZEROORMORE(OctalDigitOrUnderscore)
rule OctalDigits  : ONEOF(OctalDigit,
                    OctalDigit + ZEROORONE(OctalDigitsAndUnderscores) + OctalDigit)
rule OctalNumeral : ONEOF('0' + OctalDigits, '0' + Underscores + OctalDigits)

### Binary rules

rule BinDigit   : ONEOF('0', '1')
rule BinDigitOrUnderscore : ONEOF(BinDigit, '_')
rule BinDigitsAndUnderscores:BinDigitOrUnderscore + ZEROORMORE(BinDigitOrUnderscore)
rule BinDigits  : ONEOF(BinDigit,
                        BinDigit + ZEROORONE(BinDigitsAndUnderscores) + BinDigit)
rule BinNumeral : ONEOF("0b" + BinDigits, "0B" + BinDigits)

##########

rule IntegerTypeSuffix : ONEOF('L', 'l')

rule DecimalIntegerLiteral: DecimalNumeral + ZEROORONE(IntegerTypeSuffix)
rule HexIntegerLiteral    : HexNumeral + ZEROORONE(IntegerTypeSuffix)
rule OctalIntegerLiteral  : OctalNumeral + ZEROORONE(IntegerTypeSuffix)
rule BinaryIntegerLiteral : BinNumeral + ZEROORONE(IntegerTypeSuffix)

rule IntegerLiteral: ONEOF(DecimalIntegerLiteral,
                           HexIntegerLiteral,
                           OctalIntegerLiteral,
                           BinaryIntegerLiteral)

#########################################################################
##                       Floating Point                                ##
#########################################################################

##### Decimal floating point literal

rule Sign : ONEOF('+', '-')
rule FloatTypeSuffix : ONEOF('f', 'F', 'd', 'D')
rule ExponentIndicator : ONEOF('e', 'E')
rule SignedInteger : ZEROORONE(Sign) + Digits
rule ExponentPart : ExponentIndicator + SignedInteger

rule DecFPLiteral : ONEOF(Digits + '.' + ZEROORONE(Digits) + ZEROORONE(ExponentPart) + ZEROORONE(FloatTypeSuffix),
                    '.'+Digits + ZEROORONE(ExponentPart) + ZEROORONE(FloatTypeSuffix),
                    Digits + ExponentPart + ZEROORONE(FloatTypeSuffix),
                    Digits + ZEROORONE(ExponentPart))

####### Hex floating point literal

rule BinaryExponentIndicator : ONEOF('p', 'P')
rule BinaryExponent : BinaryExponentIndicator + SignedInteger
rule HexSignificand : ONEOF(HexNumeral + ZEROORONE('.'),
                            "0x" + ZEROORONE(HexDigits) + '.' + HexDigits,
                            "0X" + ZEROORONE(HexDigits) + '.' + HexDigits)
rule HexFPLiteral: HexSignificand + BinaryExponent + ZEROORONE(FloatTypeSuffix)

######  Floating POint Literal

rule FPLiteral : ONEOF(DecFPLiteral, HexFPLiteral)

#########################################################################
##                           Boolean                                   ##
#########################################################################

rule BooleanLiteral : ONEOF ("true", "false")

#########################################################################
##                           Character                                 ##
#########################################################################

### TODO: so far support simple character
rule CharacterLiteral : ''' + CHAR + '''

#########################################################################
##                           String                                    ##
#########################################################################

### TODO: so far support simple character

rule StringLiteral : '"' + ZEROORMORE(CHAR) + '"'

#########################################################################
##                           Null                                      ##
#########################################################################

rule NullLiteral : "null"

#########################################################################
##                           Literal                                   ##
#########################################################################

rule Literal : ONEOF(IntegerLiteral,
                     FPLiteral,
                     BooleanLiteral,
                     CharacterLiteral,
                     StringLiteral,
                     NullLiteral)
