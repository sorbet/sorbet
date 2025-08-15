# typed: strict

module IFoo
  extend T::Sig, T::Helpers
  abstract!

  sig { abstract.returns(String) }
  attr_reader :foo

  sig { abstract.params(bar: String).returns(String) }
  attr_writer :bar

  sig { abstract.returns(String) }
  attr_accessor :qux
end
