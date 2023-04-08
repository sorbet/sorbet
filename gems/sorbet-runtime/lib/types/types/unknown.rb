# frozen_string_literal: true
# typed: true

module T::Types
  # The top type
  class Unknown < Base

    def initialize; end

    # overrides Base
    def name
      "T.unknown"
    end

    # overrides Base
    def valid?(obj)
      true
    end

    # overrides Base
    private def subtype_of_single?(other)
      case other
      when T::Types::Unknown then true
      else false
      end
    end

    module Private
      INSTANCE = Unknown.new.freeze
    end
  end
end
