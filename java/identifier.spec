# So far we dont support unicode which are not major goal right now.
# [NOTE] Please refer README in AutoGen to understand the upper case
#        words.

rule JavaChar : ONEOF(CHAR, '_' , '$')
rule IDENTIFIER : JavaChar + ZEROORMORE(CHAR, DIGIT)
