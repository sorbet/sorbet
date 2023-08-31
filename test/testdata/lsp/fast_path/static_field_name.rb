# typed: true

class X
  #   ^ hover: T.class_of(X)
  extend T::Sig
  sig {params(x: Integer).void}
  def foo(
    # ^ hover: sig { params(x: Integer).void }
    # ^ hover: def foo(x); end
    x
  # ^ hover: Integer
  )
    puts(x)
    #    ^ hover: Integer
  end
end

AA = 123
#^ hover: Integer
puts AA
#    ^ hover: Integer
