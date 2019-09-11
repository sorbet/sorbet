# typed: true
class A
  C = "1"
  def self.foo
     self::C # error: Dynamic constant references are unsupported
  end
end

class B < A
  C = 2
end

T.reveal_type(A.foo) # error: T.untyped
T.reveal_type(B.foo) # error: T.untyped
