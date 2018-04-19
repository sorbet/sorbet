# typed: strict
class Identity
  extend T::Generic

  Ty = type_member

  sig(x: Ty).returns(Ty) # error: Expression does not have a fully-defined type
  def self.call(x)
    x
  end
end
