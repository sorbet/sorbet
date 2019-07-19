# typed: true
# from fuzzer: https://github.com/sorbet/sorbet/issues/1126
A = T.type_alias(A) # error: Unable to resolve right hand side of type alias `A`
C = A::B # error: Resolving constants through type aliases is not supported
