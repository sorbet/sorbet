# frozen_string_literal: true
# typed: true

module T::Types
  # Validates that an object is equal to another T::Enum singleton value.
  class TEnum < Base
    attr_reader :val

    def initialize(val)
      @val = val
    end

    # @override Base
    def name
      @val.inspect
    end

    # @override Base
    def valid?(obj)
      @val == obj
    end

    # @override Base
    private def subtype_of_single?(other)
      case other
      when TEnum
        @val == other.val
      else
        false
      end
    end
  end
end
