# frozen_string_literal: true
# typed: true

module T::Types
  class TypedRange < TypedEnumerable
    attr_reader :type

    def underlying_class
      Hash
    end

    # @override Base
    def name
      "T::Range[#{@type.name}]"
    end

    # @override Base
    def recursively_valid?(obj)
      obj.is_a?(Range) && super
    end

    # @override Base
    def valid?(obj)
      obj.is_a?(Range)
    end

    def new(*args)
      T.unsafe(Range).new(*args)
    end
  end
end
