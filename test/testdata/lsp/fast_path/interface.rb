# typed: true

module Iface
  extend T::Sig
  extend T::Helpers
  interface!
  sig { abstract.void }
  def foo; end
end

class A
  include Iface
  extend T::Sig
  sig { override.params(x: Integer).void }
  def foo(x); end # error: must accept no more than `0` required
end
