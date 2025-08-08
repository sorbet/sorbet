# typed: true

module Foo
  def foo; end
end

class A
  include Foo
end

A.foo

class Parent
  include Foo
end

class Child < Parent
end

Child.foo

module Bar
  include Foo
end

module B
  include Bar
end

B.foo

module Child1
  include Foo
end

module Child2
  include Foo
end

module Child3
  include Foo
end

class C
  include Child1, Child2, Child3
end

C.foo

module Fuga
  def foo; end
end

module Hoge
  def foo; end
end

class D
  include Foo
  prepend Fuga
  include Hoge
end

D.foo
