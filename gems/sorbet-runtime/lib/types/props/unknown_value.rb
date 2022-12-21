# frozen_string_literal: true
# typed: strict

module T::Props
  class UnknownValue
    sig {returns(T.untyped)}
    attr_reader :serialized_value

    sig {params(serialized_value: T.untyped).returns(T::Props::UnknownValue)}
    def _private_construct(serialized_value)
      @serialized_value = serialized_value
    end
  end
end