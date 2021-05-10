# frozen_string_literal: true
# typed: true

module T::Types
  # Validates that an object belongs to the specified class.
  class Simple < Base
    attr_reader :raw_type

    def initialize(raw_type)
      @raw_type = raw_type
    end

    # @override Base
    def name
      # Memoize to mitigate pathological performance with anonymous modules (https://bugs.ruby-lang.org/issues/11119)
      #
      # `name` isn't normally a hot path for types, but it is used in initializing a T::Types::Union,
      # and so in `T.nilable`, and so in runtime constructions like `x = T.let(nil, T.nilable(Integer))`.
      @name ||= @raw_type.name.freeze
    end

    # @override Base
    def valid?(obj)
      obj.is_a?(@raw_type)
    end

    # @override Base
    private def subtype_of_single?(other)
      case other
      when Simple
        @raw_type <= other.raw_type
      else
        false
      end
    end

    def to_nilable
      @nilable ||= T::Types::Union.new([self, T::Utils::Nilable::NIL_TYPE])
    end

    module Private
      module Pool
        def self.type_for_module(mod)
          cached = mod.instance_variable_get(:@__as_sorbet_type)
          return cached if cached

          type = if mod == ::Array
            T::Array[T.untyped]
          elsif mod == ::Set
            T::Set[T.untyped]
          elsif mod == ::Hash
            T::Hash[T.untyped, T.untyped]
          elsif mod == ::Enumerable
            T::Enumerable[T.untyped]
          elsif mod == ::Enumerator
            T::Enumerator[T.untyped]
          elsif mod == ::Range
            T::Range[T.untyped]
          else
            Simple.new(mod)
          end

          mod.instance_variable_set(:@__as_sorbet_type, type) unless mod.frozen?
          type
        end
      end
    end
  end
end
