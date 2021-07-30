# frozen_string_literal: true
# typed: true
# compiled: true

class Foo
  extend T::Sig

  sig {params(x: Integer, blk: T.proc.returns(Integer)).returns(Integer)}
  def bar(x, &blk)
    y = yield
    x + y
  end
end

result = Foo.new.bar(5) do
  6
end
p result
