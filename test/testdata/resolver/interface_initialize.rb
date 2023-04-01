# typed: true
module MyInterface
  extend T::Sig
  extend T::Helpers
  interface!

  sig {abstract.params(foo: T.nilable(String)).void}
  def initialize(foo); end
end

class A
  extend T::Sig
  include MyInterface

  sig {override.void}
  def initialize # error: must accept at least `1` positional
  end
end
