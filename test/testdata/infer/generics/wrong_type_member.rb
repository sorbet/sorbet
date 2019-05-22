# typed: true
class T1
  extend T::Generic

  Ty = type_member
end

class T2
  extend T::Generic
  extend T::Sig

  sig {params(x: T1::Ty).returns(T1::Ty)}
               # ^^^^^^ error: `type_member` type `T1::Ty` used outside of the class definition
                               # ^^^^^^ error: `type_member` type `T1::Ty` used outside of the class definition
  def f(x)
    x
  end
end
