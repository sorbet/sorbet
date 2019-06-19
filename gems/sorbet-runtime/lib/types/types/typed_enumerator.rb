# frozen_string_literal: true
# typed: true

module T::Types
  class TypedEnumerator < TypedEnumerable
    attr_reader :type

    # @override Base
    def name
      "T::Enumerator[#{@type.name}]"
    end

    # @override Base
    def valid?(obj)
      obj.is_a?(Enumerator) && super
    end

    def new(*args, &blk) # rubocop:disable PrisonGuard/BanBuiltinMethodOverride
      T.unsafe(Enumerator).new(*args, &blk)
    end
  end
end
