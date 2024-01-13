# typed: strict

module C
  extend T::Sig

  sig{returns(B::Bar)}
  def use_b
    T.unsafe(nil)
  end
end
