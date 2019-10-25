rule Rule1 : ONEOF(Rule2 + '+' + Rule3,
                   Rule2 + Rule4 + Rule3,
                   Rule4 + Rule5,
                   Rule6)
    attr.type : Integer
    attr.validity : IsUnsigned(%1)
    attr.validity.%1 : IsUnsigned(%3)
    attr.validity.%2 : IsUnsigned(%2); IsUnsigned(%3)
    attr.action : GenerateUnaryExpr(%1)
    attr.action.%1 : GenerateBinaryExpr(%1,%2,%3)

