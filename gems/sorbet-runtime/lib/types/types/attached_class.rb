# frozen_string_literal: true
# typed: true

module T::Types
  # Modeling AttachedClass properly at runtime would require additional
  # tracking, so at runtime we permit all values and rely on the static checker.
  # As AttachedClass is modeled statically as a type member on every singleton
  # class, this is consistent with the runtime behavior for all type members.
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
