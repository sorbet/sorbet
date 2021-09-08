# frozen_string_literal: true
# typed: true
# compiled: true

class Foo
  class << self
    def bar
      "hello"
    end
  end
end

p Foo.method_defined?(:bar)
p Foo.singleton_class.method_defined?(:bar)
puts Foo.bar
