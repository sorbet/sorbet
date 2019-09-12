# typed: true
# disable-fast-path: true
module Foo
  module_function
  def self.bar(b)
    b
  end

  def bar(a:)
    a
  end
end

puts Foo.bar(a: 1)
