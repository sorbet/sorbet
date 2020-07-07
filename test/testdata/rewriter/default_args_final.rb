# typed: true

class Parent
  extend T::Sig

  sig { params(bar: String).void }
  def void_foo(bar = ''); end

  sig { params(bar: String).returns(String) }
  def returning_foo(bar = '')
    'return'
  end
end

class Child < Parent
  extend T::Sig
  extend T::Helpers
  final!

  sig(:final) { override.params(bar: String).void }
  def void_foo(bar = ''); end

  sig(:final) { override.params(bar: String).returns(String) }
  def returning_foo(bar = '')
    'return'
  end
end

