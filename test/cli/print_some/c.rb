# typed: true

class C
  extend T::Sig

  sig {params(a: A).void}
  def cmeth(a)
  end
end
