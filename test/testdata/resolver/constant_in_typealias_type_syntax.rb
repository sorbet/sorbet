# typed: true
class Foo
  extend T::Sig

  A = T.type_alias { Integer }

  sig { returns(A::B) }
  #             ^^^^ error: Resolving constants through type aliases is not supported
  def foo; T.unsafe(nil); end

  sig { returns([A::B]) }
  #              ^^^^ error: Resolving constants through type aliases is not supported
  def bar; T.unsafe(nil); end

  x = T.let(nil, T.nilable(A::B::C))
  #                        ^^^^ error: Resolving constants through type aliases is not supported
  T.reveal_type(x) # error: `Foo::A::B::C (unresolved)`

  sig { returns({key: [A::B]}) } # error: Malformed `sig`. No method def following it
  #                    ^^^^ error: Resolving constants through type aliases is not supported
end

class Bar
  extend T::Sig

  A = T.type_alias { Integer }

  sig { params(x: A::B).void }
  #               ^^^^ error: Resolving constants through type aliases is not supported
  def foo(x); end

  T.cast(nil, A::B)
  #           ^^^^ error: Resolving constants through type aliases is not supported

  y = A::B::C
  #   ^^^^ error: Resolving constants through type aliases is not supported
  T.reveal_type(y) # error: `T.untyped`
end
