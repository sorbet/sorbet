# typed: true

class A
  extend T::Sig

  sig { type_parameters(:K).returns(T.type_parameter(:K)) } # error: The type for an `attr_reader` cannot contain `type_parameters`
  attr_reader :foo

  sig { type_parameters(:K).params(bar: T.type_parameter(:K)).returns(T.type_parameter(:K)) } # error: The type for an `attr_writer` cannot contain `type_parameters`
  attr_writer :bar

  sig { type_parameters(:K).returns(T.type_parameter(:K)) } # error: The type for an `attr_accessor` cannot contain `type_parameters`
  attr_accessor :baz
end
