# frozen_string_literal: true
# typed: true
# compiled: true

extend T::Sig

class SomethingElse
  def <=(arg0)
    "got arg: #{arg0}"
  end
end

sig {params(x: T.any(Integer, SomethingElse), y: T.untyped).returns(T.untyped)}
def do_leq(x, y)
  x <= y
end


def do_leq_untyped(x, y)
  x <= y
end


p do_leq(4, 5)
p do_leq(8, 9.0)
p do_leq(SomethingElse.new, :foo)

p do_leq_untyped(4, 5)
p do_leq_untyped(7, 3.0)
p do_leq_untyped(SomethingElse.new, [:foo])
