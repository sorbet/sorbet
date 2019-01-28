# frozen_string_literal: true
# typed: true

module T::Types
  # Takes a list of types. Validates that an object matches all of the types.
  class Intersection < Base
    attr_reader :types

    def initialize(types)
      @types = types.flat_map do |type|
        if type.is_a?(Intersection)
          # Simplify nested intersections (mostly so `name` returns a nicer value)
          type.types
        else
          T::Utils.coerce(type)
        end
      end.uniq
    end

    # @override Base
    def name
      "T.all(#{@types.map(&:name).sort.join(', ')})"
    end

    # @override Base
    def valid?(obj)
      @types.all? {|type| type.valid?(obj)}
    end

    # @override Base
    private def subtype_of_single?(other)
      raise "This should never be reached if you're going through `subtype_of?` (and you should be)"
    end

  end
end
