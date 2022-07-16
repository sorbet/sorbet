# typed: strict
extend T::Sig

class Box
  extend T::Sig
  extend T::Generic
  Elem = type_member

  sig {returns(Elem)}
  def elem; raise; end
end

class IntBox < Box
  Elem = type_member {{fixed: Integer}}
  Another = type_member

  sig {returns(Another)}
  def another; raise; end
end

sig do
  params(
    a: Box,
    #  ^^^ error: Malformed type declaration. Generic class without type arguments `Box`
    b: IntBox,
    #  ^^^^^^ error: Malformed type declaration. Generic class without type arguments `IntBox`
    c: IntBox[String],
    d: Array,
    #  ^^^^^ error: Malformed type declaration. Generic class without type arguments `Array`
  )
  .void
end
def example(a, b, c, d)
  T.reveal_type(a.elem) # error: `T.untyped`
  T.reveal_type(b.elem) # error: `Integer`
  T.reveal_type(c.another) # error: `String`
end
