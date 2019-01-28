# frozen_string_literal: true
# typed: true

module T::Types
  class TypedSet < TypedEnumerable
    attr_reader :type

    # @override Base
    def name
      "T::Set[#{@type.name}]"
    end

    # @override Base
    def valid?(obj)
      obj.is_a?(Set) && super
    end

    def new(*args) # rubocop:disable PrisonGuard/BanBuiltinMethodOverride
      Set.new(*T.unsafe(args))
    end
  end
end
