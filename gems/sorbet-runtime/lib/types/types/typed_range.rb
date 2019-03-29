# frozen_string_literal: true
# typed: true

module T::Types
  class TypedRange < TypedEnumerable
    attr_reader :type

    # @override Base
    def name
      "T::Range[#{@type.name}]"
    end

    # @override Base
    def valid?(obj)
      obj.is_a?(Range) && super
    end

    def new(*args) # rubocop:disable PrisonGuard/BanBuiltinMethodOverride
      T.unsafe(Range).new(*args)
    end
  end
end
