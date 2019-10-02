STRUCT LITERAL : ((IntegerLiteral,         Int),
                  (FPLiteral,              FP),
                  (BooleanLiteral,         Bool),
                  (CharacterLiteral,       Char),
                  (StringLiteral,          String),
                  (NullLiteral,            Null))
# 
rule Base : "hello world"
rule Try : UnKnown
rule Literal : ONEOF('1', '2')
rule Expr : ONEOF(Literal, ZEROORMORE(Expr) + ZEROORONE(Try) + "+" + Expr)

rule Literal : Rule1+Rule2

rule Stmt : ONEOF(
  ExpressionName + '=' + Expression ==> func GenerateAssignment(%1, %3),
  ExpressionName + "+=" + Expression ==> func GenerateAddAssignment(%1, %3),
  ExpressionName + "-=" + Expression ==> func GenerateSubAssignment(%1, %3))

rule Expression : ONEOF(
  Primary,
  ExpressionName,
  Expression + '+' + Expression ==> func GenerateBinaryExpr(%1, %2, %3),
  Expression + '-' + Expression ==> func GenerateBinaryExpr(%1, %2, %3))
