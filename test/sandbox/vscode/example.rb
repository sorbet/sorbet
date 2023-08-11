# typed: strict

class A
  extend T::Sig
  sig {params(x: Integer).void}
  def example(x)
    if x.even?
      puts(x)
    end
  end
end
