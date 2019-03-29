# frozen_string_literal: true
# typed: true

module T::Types
  # Validates that an object belongs to the specified class.
  class ClassOf < Base
    attr_reader :type

    def initialize(type)
      @type = type
    end

    # @override Base
    def name
      "T.class_of(#{@type})"
    end

    # @override Base
    def valid?(obj)
      obj.is_a?(Module) && obj <= @type
    end

    # @override Base
    def subtype_of_single?(other)
      case other
      when ClassOf
        @type <= other.type
      else
        false
      end
    end

    # @override Base
    def describe_obj(obj)
      obj.inspect
    end
  end
end
