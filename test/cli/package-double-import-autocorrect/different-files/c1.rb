# typed: strict

module C
  extend T::Sig

  sig{returns(A::Foo)}
  def first_reference
    T.unsafe(nil)
  end
end
