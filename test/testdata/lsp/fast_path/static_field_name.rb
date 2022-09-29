# typed: true

class X
  #   ^ hover: T.class_of(X)
  # ___________________________________________
  extend T::Sig
  sig {params(x: Integer).void}
  def foo(
    # ^ hover: sig {params(x: Integer).void}
    # ^ hover: def foo(x); end
    # _________________________________________
    x
  # ^ hover: Integer
  # _________________________________________
  )
    puts(x)
    #    ^ hover: Integer
    #    _________________________________________
  end
end

AA = 123
#^ hover: Integer
#_________________________________________
puts AA
#    ^ hover: Integer
#    _________________________________________
