# typed: false

puts(T.unsafe(nil)::A)

puts(T.unsafe(nil)::A::B)
#    ^^^^^^^^^^^^^^^^^^^ error: Unable to resolve constant `B`

AliasToInt = T.type_alias { Integer }

puts(AliasToInt::X)
#    ^^^^^^^^^^^^^ error: Resolving constants through type aliases is not supported

puts(AliasToInt::X::Y)
#    ^^^^^^^^^^^^^ error: Resolving constants through type aliases is not supported
#    ^^^^^^^^^^^^^^^^ error: Unable to resolve constant `Y`
