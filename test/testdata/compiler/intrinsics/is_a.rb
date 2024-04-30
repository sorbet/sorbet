# frozen_string_literal: true
# typed: true
# compiled: true

class Normal
  def f(x, y)
    x.is_a?(y)
  end

  def g(x, y)
    x.kind_of?(y)
  end
end



class Override
  def is_a?(x)
    "always"
  end

  def kind_of?(x)
    "definitely"
  end
end

n = Normal.new

p n.f("a", String)
p n.f(1, Integer)
p n.f([], Hash)

p n.g("a", String)
p n.g(1, Integer)
p n.g([], Hash)

o = Override.new

p n.f(o, String)
p n.f(o, Integer)
p n.g(o, Array)
p n.g(o, Hash)
