# typed: true
extend T::Sig

module A
  class B
  end
end

sig {params(x: T.untyped).void}
def foo(x)
  case x.class
  when Integer
     # ^ hover: T.class_of(Integer)
    1
  when A::B
     # ^ hover: T.class_of(A)
        # ^ hover: T.class_of(A::B)
    2
  end
end
