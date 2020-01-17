# typed: true
# compiled: true

class A
  extend T::Sig
  sig {params(raise_on_error: Integer).void}
  def self.foo(raise_on_error: 0)
    puts raise_on_error
  end
end

A.foo
