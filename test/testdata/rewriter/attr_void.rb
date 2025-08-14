# typed: true

class A
  extend T::Sig

  sig { void }
#       ^^^^ error: An `attr_reader` cannot be `void`
  attr_reader :bad_reader

  sig { void }
#       ^^^^ error: An `attr_accessor` cannot be `void`
  attr_accessor :bad_writer
end
