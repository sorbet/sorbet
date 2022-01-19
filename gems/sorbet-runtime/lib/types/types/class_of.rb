# frozen_string_literal: true
# typed: true

module T::Types
  # Validates that an object belongs to the specified class.
  class ClassOf < Base
    attr_reader :type

    def initialize(type)
      @type = type
    end

    # overrides Base
    def name
      "T.class_of(#{@type})"
    end

    # overrides Base
    def valid?(obj)
      obj.is_a?(Module) && obj <= @type
    end

    # overrides Base
    def subtype_of_single?(other)
      case other
      when ClassOf
        @type <= other.type
      when Simple
        @type.is_a?(other.raw_type)
      else
        false
      end
    end

    # overrides Base
    def describe_obj(obj)
      obj.inspect
    end
  end
end
