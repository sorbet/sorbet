# frozen_string_literal: true
# typed: true

module T::Private::Types
  # Wraps a proc for a type alias to defer its evaluation.
  class Union < T::Private::Types::TypeAlias

    def initialize(type_a, type_b, *types)
      @type_a = type_a
      @type_b = type_b
      @types = types
    end

    def aliased_type
      @aliased_type ||= begin
        @type_a = T::Utils.coerce(@type_a)
        @type_b = T::Utils.coerce(@type_b)
        @types = @types.map {|t| T::Utils.coerce(t)} if !@types.empty?
        T::Types::Union::Private::Pool.union_of_types(@type_a, @type_b, @types)
      end
    end
  end
end
