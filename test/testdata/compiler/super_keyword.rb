# frozen_string_literal: true
# typed: true
# compiled: true

class Parent
  def initialize(hash={})
    puts "parent: hash=#{hash}"
  end
end

class Child < Parent
  def initialize(foo:)
    puts "child: foo=#{foo}"
    super
  end
end

Child.new(foo: 0)
