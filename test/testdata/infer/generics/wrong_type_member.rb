# typed: true
class T1
  extend T::Generic

  Ty = type_member
end

class T2
  extend T::Generic
  extend T::Sig

  sig {params(x: T1::Ty).returns(T1::Ty)}
  def f(x) # error: Expression does not have a fully-defined type
    x
  end
end
