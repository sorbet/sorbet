# typed: strict

class A
  extend T::Sig
  extend T::Generic
  Elem = type_template
end

class Test
  extend T::Sig

  sig {params(arg: T.class_of(A)).void}
  def arg_test(arg)
  end

  # This exercises the parsing of `T.class_of(X)` where `X` has type_template
  # members, ensuring that we're calling `externalType` on the singleton instead
  # of just  making a `ClassType`.
  sig {params(arg: T.class_of(A)).void}
  def test(arg)
    if arg < A
      T.reveal_type(arg) # error: Revealed type: `T.class_of(A)[A, T.untyped]`
    end

    # Exercise the `Object#class` intrinsic, ensuring that it calls
    # `externalType` instead of just making a `ClassType` of the singleton.
    self.arg_test(A.new.class)
  end

end
