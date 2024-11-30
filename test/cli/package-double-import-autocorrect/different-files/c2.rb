# typed: strict

module C
  extend T::Sig

  sig{returns(A::Foo)}
  def second_reference
    T.unsafe(nil)
  end
end
