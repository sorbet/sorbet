# frozen_string_literal: true
# typed: true

module T::Types
  # Validates that an object belongs to the specified class.
  class Simple < Base
    attr_reader :raw_type

    def initialize(raw_type)
      @raw_type = raw_type
    end

    # @override Base
    def name
      @raw_type.name
    end

    # @override Base
    def valid?(obj)
      obj.is_a?(@raw_type)
    end

    # @override Base
    private def subtype_of_single?(other)
      case other
      when Simple
        @raw_type <= other.raw_type
      else
        false
      end
    end
  end
end
