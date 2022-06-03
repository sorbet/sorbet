# typed: true

class A
  extend T::Sig
  sig {params(x: Integer).void}
  def foo(x)
    # ^ hover: sig {params(x: Integer).void}
    # ^ hover: def foo(x); end
  end
end
