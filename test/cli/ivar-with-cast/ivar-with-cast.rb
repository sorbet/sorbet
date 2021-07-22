# typed: true

class A
  extend T::Sig
  def initialize
    @val = T.cast(0, Integer)
  end
end
