# frozen_string_literal: true
# typed: true

module T::Types
  # Modeling AttachedClass properly at runtime would require additional
  # tracking, so at runtime we permit all values and rely on the static checker.
  class AttachedClassType < Base

    def initialize(); end

    # @override Base
    def name
      "AttachedClass"
    end

    # @override Base
    def valid?(obj)
      true
    end

    # @override Base
    private def subtype_of_single?(other)
      case other
      when AttachedClassType
          true
        else
          false
      end
    end
  end
end

AttachedClass = T.unsafe(T::Types::AttachedClassType.new)
