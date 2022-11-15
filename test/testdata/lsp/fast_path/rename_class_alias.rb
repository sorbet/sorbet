# typed: true

class A
  def self.foo; end
end

B = A

B.foo
C.foo # error: Unable to resolve constant `C`
