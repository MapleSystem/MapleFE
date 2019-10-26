###################################################################################
# This file defines the Java types.
#
#  1. Child rules have to be defined before parent rules.
#  2. For type.spec, Autogen allows the programmer to have special keyword section to
#     define the individual <keyword, meaning>. Please keep in mind this
#     keyword section has to appear before the rule section.
#
#     Keyword section is defined through STRUCT().
#
#  3. Autogen will first read in the keyword section by TypeGen functions, then
#     read the rules section by inheriting BaseGen::Parse, etc.
#  4. The keyword duplex is defined as <TYPE, "keyword">.
#     [NOTE] TYPE should be one of those recognized by the "parser". Please refer
#            to ../shared/include/type.h. The data representation of same type
#            doesnt have to be the same in each language, we just need have the
#            same name of TYPE. The physical representation of each type of
#            different language will be handled by HandleType() by each language.
#            e.g. Char in java and in C are different. but Autogen doesnt care.
#            java2mpl and c2mpl will provide their own HandleType() to map this
#            to the correct types in Maple IR.
#
#     So there are 4 type systems involved in frontend.
#     (1) Types in the .spec file, of each language
#     (2) Types in Autogen. Super set of (1) in all languages.
#         types in Autogen have an exact mapping to those in Parser.
#         Autogen will generate type related files used in parser, mapping
#         types to it.
#     (3) Types in Parser. From now on, physical representation is defined.
#         Each language has its own HandleTypes() to map type in Parser to
#         those in MapleIR, considering physical representation.
#     (4) Types in Maple IR. This is the only place where types have physical
#         representation.
#
#  5. The supported STRUCT in type.spec is:
#         Keyword
#     Right now, only one STRUCT is supported.
#
#     It is highly possible that a .spec file needs more than one STRUCT.
###################################################################################

# The types recoganized by Autogen are in autogen/supported_types.spec
# where Boolean, Byte, .. are defined. That said, "Boolean" and the likes are
# used in type.spec as a keyword.
#
# This STRUCT tells the primitive types

STRUCT Keyword : (("boolean", Boolean),
                  ("byte", Byte),
                  ("short", Short),
                  ("int", Int),
                  ("long", Long),
                  ("char", Char),
                  ("float", Float),
                  ("double", Double),
                  ("null", Null))

###################################################################################
#                                                                                 #
###################################################################################

rule BoolType: Boolean
rule IntType : ONEOF(Byte, Short, Int, Long, Char)
rule FPType  : ONEOF(Float, Double)
rule NumericType : ONEOF(IntType, FPType)

# this is a fake one, to make the development work.
rule Annotation : "annotation"

rule PrimitiveType : ONEOF( ZEROORMORE(Annotation) + NumericType,
                       ZEROORMORE(Annotation) + BoolType )

############################
# special Null type in java.
############################

rule NullType : Null

###################################################################################
#                             Reference Types                                     #
###################################################################################

# TypeArguments are for generic types
rule WildcardBounds   : ONEOF( "extends" + ReferenceType, "super" + ReferenceType)
rule Wildcard         : ZEROORMORE(Annotation) + '?' + ZEROORONE(WildcardBounds)
rule TypeArgument     : ONEOF(ReferenceType, Wildcard)
rule TypeArgumentList : TypeArgument + ZEROORMORE(',' + TypeArgument)
rule TypeArguments    : '<' + TypeArgumentList + '>'

rule ClassType : ONEOF( ZEROORMORE(Annotation) + Identifier + ZEROORONE(TypeArguments),
                        ClassOrInterfaceType + '.' + ZEROORMORE(Annotation) + Identifier
                           + ZEROORONE(TypeArguments))
rule InterfaceType : ClassType
rule TypeVariable  : ZEROORMORE(Annotation) + Identifier
rule ArrayType     : ONEOF( PrimitiveType + Dims,
                            ClassOrInterfaceType + Dims,
                            TypeVariable + Dims )
rule ClassOrInterfaceType : ONEOF(ClassType, InterfaceType)
rule ReferenceType        : ONEOF(ClassOrInterfaceType, TypeVariable, ArrayType)

###########################
#  Final one
###########################
rule TYPE: ONEOF(PrimitiveType, ReferenceType, NullType)

#####################################################################################
#                       Abnormal types                                              #
#####################################################################################

rule UnannClassType: ONEOF(Identifier + ZEROORONE(TypeArguments),
        UnannClassOrInterfaceType + '.' + ZEROORMORE(Annotation) + Identifier + ZEROORONE(TypeArguments))
rule UnannInterfaceType : UnannClassType
rule UnannTypeVariable : Identifier

# UnannArrayType no implemented yet
#rule UnannReferenceType: ONEOF(UnannClassOrInterfaceType, UnannTypeVariable, UnannArrayType)
rule UnannReferenceType: ONEOF(UnannClassOrInterfaceType, UnannTypeVariable)

rule UnannPrimitiveType: ONEOF(NumericType, Boolean)
rule UnannType: ONEOF(UnannPrimitiveType, UnannReferenceType)

rule UnannClassOrInterfaceType: ONEOF(UnannClassType, UnannInterfaceType)

# Dims is not implemented yet
#rule UnannArrayType : ONEOF(UnannPrimitiveType + Dims,
#                            UnannClassOrInterfaceType + Dims,
#                            UnannTypeVariable + Dims)
