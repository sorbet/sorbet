# typed: true
# selective-apply-code-action: refactor.rewrite
extend T::Sig

class A
  extend T::Sig

  sig {void}
  def nullary
    # | apply-code-action: [A] Convert to singleton class method (best effort)
    puts "Hello, peter."
  end

  def example
    nullary
    nullary()
    nullary {}
    nullary() {}
    nullary do
    end
    nullary() do
    end
  end

  sig {params(other: A).returns(T.self_type)}
  def +(other); self; end
end

A.new.nullary do
end
A.new.nullary {}
A.new.nullary() do
end

(T.unsafe(A.new)).nullary

(A.new + A.new).nullary
