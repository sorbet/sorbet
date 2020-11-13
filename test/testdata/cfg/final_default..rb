# typed: true
module Iface
  extend T::Sig
  extend T::Helpers
  interface!

  sig do
    abstract
    .params(
      bar: String,
    ).void
  end
  def foo(bar=''); end
end

class A
  extend T::Sig
  extend T::Helpers
  include Iface
  final!

  sig(:final) do
    override
    .params(
      bar: String
    ).void
  end
  def foo(
    # This used to cause an error when we had the DefaultArgs pass
    bar=''
  ); end

  sig(:final) do
    params(
      bar: String
    ).void
  end
  def non_interface_method(
    bar=''
  ); end
end
