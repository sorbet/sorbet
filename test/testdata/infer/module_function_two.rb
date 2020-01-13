# typed: true

module Foo
  module_function
  def self.bar(b)
    b
  end

  def bar(a:) # error: Method `Foo.bar` redefined with argument `a` as a keyword argument
    a
  end
end

puts Foo.bar(a: 1)
