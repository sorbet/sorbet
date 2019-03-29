# frozen_string_literal: true
# typed: true

module T::Types
  class TypedArray < TypedEnumerable
    # @override Base
    def name
      "T::Array[#{@type.name}]"
    end

    # @override Base
    def valid?(obj)
      obj.is_a?(Array) && super
    end

    def new(*args) # rubocop:disable PrisonGuard/BanBuiltinMethodOverride
      Array.new(*T.unsafe(args))
    end
  end
end
