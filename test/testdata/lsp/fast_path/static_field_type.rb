# typed: true

class A
  def foo(x); end

  X = 1
end

T.reveal_type(A::X) # error: `Integer`
