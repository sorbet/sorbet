# typed: strict
# typed: strict

class Minimal
  extend T::Generic
  extend T::Sig

  A = type_member
  B = type_member {{ fixed: T.nilable(A) }}

  sig { returns(A) }
  def a = T.unsafe(nil)

  sig { returns(B) }
  def b = T.unsafe(nil)
end

g = Minimal[Integer].new
T.reveal_type(g.a) # error: Integer
T.reveal_type(g.b) # error: T.nilable(Integer)

class Backward
  extend T::Generic
  extend T::Sig

  A = type_member {{ fixed: T.nilable(B) }}
  B = type_member

  sig { returns(A) }
  def a = T.unsafe(nil)

  sig { returns(B) }
  def b = T.unsafe(nil)
end

g = Backward[Integer].new
T.reveal_type(g.a) # error: T.untyped
T.reveal_type(g.b) # error: Integer
