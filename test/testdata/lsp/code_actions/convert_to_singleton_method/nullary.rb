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
    #       ^^ error: does not take a block
    nullary() {}
    #         ^^ error: does not take a block
    nullary do # error: does not take a block
    end
    nullary() do # error: does not take a block
    end
  end

  sig {params(other: A).returns(T.self_type)}
  def +(other); self; end
end

A.new.nullary do # error: does not take a block
end
A.new.nullary {}
#             ^^ error: does not take a block
A.new.nullary() do # error: does not take a block
end

(T.unsafe(A.new)).nullary

(A.new + A.new).nullary
