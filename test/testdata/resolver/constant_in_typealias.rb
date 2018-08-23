# typed: true
module A
  B = 1
end
Alias = T.type_alias(A)
Alias::B # error: Resolving constants through type aliases is not supported

