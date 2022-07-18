# typed: true
class ParentBox
  extend T::Sig
  extend T::Generic
  Elem = type_member

  sig {returns(T.attached_class)}
  def self.make
    x = new
    T.reveal_type(x) # error: `T.attached_class (of ParentBox)`
    x
  end
end

class ChildBox < ParentBox
  Elem = type_member
end

T.reveal_type(ParentBox.new)           # error: `ParentBox[T.untyped]`
T.reveal_type(ParentBox[Integer].make) # error: `ParentBox[Integer]`

T.reveal_type(ChildBox.new)           # error: `ChildBox[T.untyped]`
T.reveal_type(ChildBox[Integer].make) # error: `ChildBox[Integer]`
