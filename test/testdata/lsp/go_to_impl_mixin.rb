# typed: true

module IFoo
  extend T::Sig
  extend T::Helpers
  interface!

  sig {abstract.void}
  def foo; end
  #   ^ find-implementation: foo
end

module FooImpl
  extend T::Sig
  sig {void}
  def foo
  #   ^ implementation: foo
    Kernel.puts 'Concrete foo impl'
  end
end

class Foo
  include FooImpl
  include IFoo
end
