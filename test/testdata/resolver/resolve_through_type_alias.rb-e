# typed: true

A = T.type_alias(T.any(String,Integer))
C = A::B
#   ^^^^ error: Resolving constants through type aliases is not supported
C = A
#   ^ error: Reassigning a type alias is not allowed
