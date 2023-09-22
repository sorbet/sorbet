# typed: true

class A
  extend T::Sig
  extend T::Generic

  Member = type_member
  Template = type_template

  sig {returns(Template)}
  def foo; end

  sig {returns(Member)}
  def self.foo; end
end

# typed: true

module IFoo
  extend T::Generic
  X = type_member
end

class Foo1
  extend T::Generic
  extend IFoo
  X = type_member
end

class Foo2
  extend T::Generic
  include IFoo
  X = type_template
end

class Foo3
  include IFoo
  class << self
    extend T::Generic
    X = type_member
  end
end
