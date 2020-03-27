# typed: strong
module I1
  extend T::Sig
  sig {void}
  def i1; end
end

module I2
  extend T::Sig
  sig {void}
  def i2; end
end

extend T::Sig
sig {params(x: T.all(I1, I2)).void}
def foo(x)
  x.i1
  x.i2
end
