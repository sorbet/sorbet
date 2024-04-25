# frozen_string_literal: true
# typed: true
# compiled: true

class IFoo
  extend T::Sig
  extend T::Helpers
  abstract!

  sig {abstract.void}
  def self.foo; end
end


class Foo < IFoo

  sig {override.void}
  def self.foo
    Kernel.p "Override!"
  end
end

p Foo.foo
