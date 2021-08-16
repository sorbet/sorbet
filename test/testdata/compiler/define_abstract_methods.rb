# frozen_string_literal: true
# typed: true
# compiled: true

class Module
  include T::Sig
end

module Impl
  sig {void}
  def foo
    Kernel.puts 'hello'
  end
end

module Interface
  extend T::Helpers
  abstract!

  p respond_to?(:foo)

  sig {abstract.void}
  def foo; end
end

class A
  include Impl
  include Interface
end

p A.ancestors
a = A.new
p a.method(:foo).owner
a.foo
