# typed: true

extend T::Sig

class C
  @@x = T.let(42, Integer)

  def f
    @@x ||= 43 # error: This code is unreachable
  end

  def self.g
    @@x ||= 44 # error: This code is unreachable
  end
end
