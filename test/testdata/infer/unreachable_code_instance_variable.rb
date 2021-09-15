# typed: true

extend T::Sig

class C
  def initialize
    @x = T.let(42, Integer)
  end

  def f
    @x ||= 43 # error: This code is unreachable
  end
end
