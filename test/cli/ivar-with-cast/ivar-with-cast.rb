# typed: true

class A
  extend T::Sig
  def initialize
    @val = T.cast(0, Integer)
  end
end

class B
  extend T::Sig

  sig {params(v: T.nilable(Integer)).void}
  def initialize(v)
    @val = T.must(v)
  end
end
