# typed: true

module M
end

class A
  extend T::Generic
  Elem = type_member

  include M
end

class Test
  extend T::Sig

  sig {params(x: T.any(A[Integer], M)).void}
  def test(x)
    T.reveal_type(x) # error: Revealed type: `M`
  end
end
