rule ClassDeclaration : ONEOF(NormalClassDeclaration, EnumDeclaration)
rule NormalClassDeclaration : ZEROORMORE(ClassModifier) + "class" + Identifier +
                              ZEROORONE(TypeParameters) + ZEROORONE(Superclass) +
                              ZEROORONE(Superinterfaces) + ClassBody

rule ClassModifier : ONEOF(Annotation, "public", "protected", "private", "abstract",
                           "static", "final", "strictfp")

# 1. Generic class
# 2. TypeParameter will be defined in type.spec
#    I just put a fake one here. Will come back to finish it
rule TypeParameters    : '<' + TypeParameterList + '>'
rule TypeParameterList : TypeParameter + ZEROORMORE(',' + TypeParameter)
rule TypeParameter     : "typeparameter"

# ClassType and InterfaceType are defined in type.spec
rule SuperClass        : "extends" + ClassType
rule Superinterfaces   : "implements" + InterfaceTypeList
rule InterfaceTypeList : InterfaceType + ZEROORMORE(',' + InterfaceType)

# class body
rule ClassBody              : "{" + ZEROORMORE(ClassBodyDeclaration) + "}"
rule ClassBodyDeclaration   : ONEOF(ClassMemberDeclaration)
#                                    InstanceInitializer,
#                                    StaticInitializer,
#                                    ConstructorDeclaration)
rule ClassMemberDeclaration : ONEOF(FieldDeclaration)
#                                    MethodDeclaration,
#                                    ClassDeclaration,
#                                    InterfaceDeclaration,
#                                    ';')
rule FieldDeclaration       : ZEROORMORE(FieldModifier) + UnannType + VariableDeclaratorList + ';'
# A fake one
rule FieldModifier          : "fakefieldmodifier"
