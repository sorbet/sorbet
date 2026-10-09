# typed: true
class Foo
  A = T.type_alias { Integer }

  class Child < A::B; end
  #             ^^^^ error: Resolving constants through type aliases is not supported

  include A::B
  #       ^^^^ error: Resolving constants through type aliases is not supported
  #       ^^^^ error: Resolving constants through type aliases is not supported
end
