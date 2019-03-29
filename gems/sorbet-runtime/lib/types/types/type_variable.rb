# frozen_string_literal: true
# typed: true

module T::Types
  # Since we do type erasure at runtime, this just validates the variance and
  # provides some syntax for the static type checker
  class TypeVariable < Base
    attr_reader :variance

    VALID_VARIANCES = [:in, :out, :invariant]

    def initialize(variance)
      if !VALID_VARIANCES.include?(variance)
        raise TypeError.new("invalid variance #{variance}")
      end
      @variance = variance
    end

    def valid?(obj)
      true
    end

    def subtype_of_single?(type)
      true
    end

    def name
      Untyped.new.name
    end
  end
end
