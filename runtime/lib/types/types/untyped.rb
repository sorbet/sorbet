# frozen_string_literal: true
# typed: true

module T::Types
  # A dynamic type, which permits whatever
  class Untyped < Base

    def initialize; end

    # @override Base
    def name
      "T.untyped"
    end

    # @override Base
    def valid?(obj)
      true
    end

    # @override Base
    private def subtype_of_single?(other)
      true
    end
  end
end
