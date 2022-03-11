# typed: true
class Identity
  extend T::Generic
  extend T::Sig

  Ty = type_member

  sig {params(x: Ty).returns(Ty)}
               # ^^ error: `type_member` type `Ty` used in a singleton method definition
                           # ^^ error: `type_member` type `Ty` used in a singleton method definition
  def self.call(x)
    x
  end
end
