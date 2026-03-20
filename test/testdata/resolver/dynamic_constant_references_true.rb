# typed: true

puts(T.unsafe(nil)::A)
#    ^^^^^^^^^^^^^^^^ error: Dynamic constant references are unsupported

puts(T.unsafe(nil)::A::B)
#    ^^^^^^^^^^^^^^^^ error: Dynamic constant references are unsupported

AliasToStr = T.type_alias { String }

puts(AliasToStr::X)
#    ^^^^^^^^^^^^^ error: Resolving constants through type aliases is not supported

puts(AliasToStr::X::Y)
#    ^^^^^^^^^^^^^ error: Resolving constants through type aliases is not supported
