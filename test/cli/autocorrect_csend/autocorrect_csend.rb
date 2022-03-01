# typed: true

extend T::Sig

class A
  extend T::Sig
  sig {returns(T.nilable(String))}
  def foo; end
end

sig {params(x: String).void}
def use_string(x)
end

maybe_a = T.let(A.new, T.nilable(A))
use_string(maybe_a&.foo)
