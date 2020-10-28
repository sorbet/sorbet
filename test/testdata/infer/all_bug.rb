# typed: true

module M; end

class A; include M; end
class B < A; end

class Test
  extend T::Sig

  sig do
    type_parameters(:T)
      .params(x: T.all(M, T.type_parameter(:T)))
      .returns(T.all(M, T.type_parameter(:T)))
  end
  def self.test(x)
    x
  end
end

T.assert_type!(Test.test(T.unsafe(nil)), M)
T.assert_type!(Test.test(A.new), A)
T.assert_type!(Test.test(B.new), B)
