# frozen_string_literal: true
# typed: strict

module T::Props
  class UnknownValue
    extend T::Sig
    extend T::Helpers

    sig {returns(T.untyped)}
    attr_reader :serialized_value

    sig {returns(StandardError)}
    attr_reader :error

    sig {params(serialized_value: T.untyped, error: StandardError).void}
    def initialize(serialized_value, error)
      @serialized_value = T.let(serialized_value, T.untyped)
      @error = T.let(error, StandardError)
    end
  end
end
