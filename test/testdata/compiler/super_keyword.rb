# frozen_string_literal: true
# typed: true
# compiled: true

class Parent
  def initialize(hash={})
    puts "parent: hash=#{hash}"
  end
end

# zero-argument super calls should be able to call private methods
class Child < Parent
  def initialize(foo:)
    puts "child: foo=#{foo}"
    super
  end
end

# explicit calls to super should be able to call private methods
class ChildExplicitCall < Parent
  def initialize(foo:)
    puts "child explicit call: foo=#{foo}"
    super(foo: foo)
  end
end

Child.new(foo: 0)
ChildExplicitCall.new(foo: 19)
