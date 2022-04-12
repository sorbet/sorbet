# typed: true
class A
  def self.foo
  end

  def bar
    foo
  end
end

A.new.foo

class Left
  def self.on_both; end
end
class Right
  def self.on_both; end
end

left_or_right = T.let(Left.new, T.any(Left, Right))
left_or_right.on_both
