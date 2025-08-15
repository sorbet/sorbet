# typed: strict

module IFoo
  extend T::Sig, T::Helpers
  abstract!

  sig { abstract.returns(String) }
  attr_reader :foo # error: Abstract methods must not contain any code in their body

  sig { abstract.params(bar: String).returns(String) }
  attr_writer :bar # error: Abstract methods must not contain any code in their body

  sig { abstract.returns(String) }
  attr_accessor :qux # error: Abstract methods must not contain any code in their body
end
