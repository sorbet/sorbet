# typed: true
class Identity
  extend T::Generic
  extend T::Sig

  Ty = type_member

  sig {params(x: Ty).returns(Ty)}
  def self.call(x) # error: Expression does not have a fully-defined type
    x
  end
end
