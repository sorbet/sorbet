# typed: true
# selective-apply-code-action: refactor.rewrite
extend T::Sig

class A
  extend T::Sig

  sig {params(x: Integer).void}
  def unary(x)
    # ^ apply-code-action: [A] Convert to singleton class method (best effort)
    puts "Hello, peter."
  end

  def example
    unary # error: Not enough arguments provided
    unary(0)
  end

  sig {params(other: A).returns(T.self_type)}
  def +(other); self; end
end

A.new.unary # error: Not enough arguments provided
A.new.unary(0)
A.new.unary(
  0
)

(T.unsafe(A.new)).unary(0)

(A.new + A.new).unary(0)
