# frozen_string_literal: true
# typed: true
# compiled: true

extend T::Sig

class SomethingElse
  def !=(arg0)
    "got arg: #{arg0}"
  end
end

sig {params(x: T.any(Integer, SomethingElse), y: T.untyped).returns(T.untyped)}
def do_neq(x, y)
  x != y
end


def do_neq_untyped(x, y)
  x != y
end


p do_neq(4, 5)
p do_neq(8, 9.0)
p do_neq(SomethingElse.new, :foo)

p do_neq_untyped(4, 5)
p do_neq_untyped(7, 3.0)
p do_neq_untyped(SomethingElse.new, [:foo])
