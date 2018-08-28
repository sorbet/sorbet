# typed: true
class A
  def self.foo
  end

  def bar
    foo
  end
end

A.new.foo
