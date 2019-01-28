# frozen_string_literal: true
# typed: true

module T::Types
  # The bottom type
  class NoReturn < Base

    def initialize; end

    # @override Base
    def name
      "T.noreturn"
    end

    # @override Base
    def valid?(obj)
      false
    end

    # @override Base
    private def subtype_of_single?(other)
      true
    end
  end
end
