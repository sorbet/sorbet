# typed: true

class X
  #   ^ hover: T.class_of(X)
  extend T::Sig
  sig {params(x: Integer).void}
  def foo(x)
    # ^ hover: sig {params(x: Integer).void}
    # ^ hover: def foo(x); end
    #     ^ hover: Integer
    puts(x)
    #    ^ hover: Integer
  end
end

AA = nil
#____________
puts AA
#________________
