# frozen_string_literal: true
# typed: true

module T::Types
  # validates that the provided value is within a given set/enum
  class Enum < Base
    extend T::Sig

    attr_reader :values
    attr_reader :method_name

    # TODO Ideally Hash would not be accepted but there are a lot of uses with prop enum.
    sig {params(values: T.any(Array, Set, Hash, T::Range[T.untyped]), method_name: String).void}
    def initialize(values, method_name: "enum")
      @values = values
      @method_name = method_name
    end

    # overrides Base
    def valid?(obj)
      @values.member?(obj)
    end

    # overrides Base
    private def subtype_of_single?(other)
      case other
      when Enum
        (other.values - @values).empty?
      else
        false
      end
    end

    # overrides Base
    def name
      "T.#{@method_name}([#{@values.map(&:inspect).join(', ')}])"
    end

    # overrides Base
    def describe_obj(obj)
      obj.inspect
    end
  end
end
