# frozen_string_literal: true
# typed: true

module T::Types
  # Takes a list of types. Validates that an object matches at least one of the types.
  class Union < Base
    attr_reader :types

    def initialize(types)
      @types = types.flat_map do |type|
        if type.is_a?(Union)
          # Simplify nested unions (mostly so `name` returns a nicer value)
          type.types
        else
          T::Utils.coerce(type)
        end
      end.uniq
    end

    # @override Base
    def name
      type_shortcuts(@types)
    end

    private def type_shortcuts(types)
      if types.size == 1
        return types[0].name
      end
      nilable = T::Utils.coerce(NilClass)
      trueclass = T::Utils.coerce(TrueClass)
      falseclass = T::Utils.coerce(FalseClass)
      if types.any? {|t| t == nilable}
        remaining_types = types.reject {|t| t == nilable}
        "T.nilable(#{type_shortcuts(remaining_types)})"
      elsif types.any? {|t| t == trueclass} && types.any? {|t| t == falseclass}
        remaining_types = types.reject {|t| t == trueclass || t == falseclass}
        type_shortcuts([T::Private::Types::StringHolder.new("T::Boolean")] + remaining_types)
      else
        names = types.map(&:name).compact.sort
        "T.any(#{names.join(', ')})"
      end
    end

    # @override Base
    def valid?(obj)
      @types.each do |type|
        return true if type.valid?(obj)
      end

      false
    end

    # @override Base
    private def subtype_of_single?(other)
      raise "This should never be reached if you're going through `subtype_of?` (and you should be)"
    end

  end
end
