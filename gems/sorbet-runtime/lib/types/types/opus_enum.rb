# frozen_string_literal: true
# typed: true

module T::Types
  # Validates that an object is equal to another Opus::Enum singleton value.
  class OpusEnum < Base
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
      when OpusEnum
        @val == other.val
      else
        false
      end
    end
  end
end
