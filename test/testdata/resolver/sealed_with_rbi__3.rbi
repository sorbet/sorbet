# typed: true

module M
  extend T::Helpers
end

class A
  include M
  def from_a; end
end

class B
  include M
  def from_b; end
end

class C
  include M
  def from_c; end
end

class D
  include M # error-with-dupes: `M` is sealed and cannot be included in `D`
  def from_d; end
end
