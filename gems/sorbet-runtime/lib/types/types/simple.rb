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
      @raw_type.name
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
          cached = mod.instance_variable_get(:@__as_sorbet_simple_type)
          return cached if cached

          type = Simple.new(mod)
          mod.instance_variable_set(:@__as_sorbet_simple_type, type) unless mod.frozen?
          type
        end
      end
    end
  end
end
