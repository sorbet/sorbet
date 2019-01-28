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
      if @types.size == 1
        return @types[0].name
      end
      nilable = T::Utils.coerce(NilClass)
      if @types.any? {|t| t == nilable}
        remaining_types = @types.reject {|t| t == nilable}
        if remaining_types.length == 1
          "T.nilable(#{remaining_types.first.name})"
        else
          "T.nilable(T.any(#{pretty_names(remaining_types)}))"
        end
      else
        "T.any(#{pretty_names(@types)})"
      end
    end

    private def pretty_names(names)
      types = names.map(&:name).sort
      if types.length == 2 && types[0] == 'FalseClass' && types[1] == 'TrueClass'
        types = ['TrueClass', 'FalseClass']
      end
      types.join(", ")
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
