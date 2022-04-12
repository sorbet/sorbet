# typed: true
class A
  def foo
  end
  def self.bar
  end

  attr_reader :my_attribute
  def  too_many_spaces; end
end

A.foo
A.new.bar
A.my_attribute
A.too_many_spaces
