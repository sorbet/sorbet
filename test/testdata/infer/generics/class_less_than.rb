# typed: strict

class A
  extend T::Sig
  extend T::Generic

  Elem = type_template
end

class Test
  extend T::Sig

  sig {params(arg: T.any(T.class_of(A), T.class_of(Integer))).void}
  def test(arg)
    if arg < A
      T.cast(arg, T.class_of(A)) # error: Useless cast
    end
  end
end
