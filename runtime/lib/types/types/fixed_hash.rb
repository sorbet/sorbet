# frozen_string_literal: true
# typed: true

module T::Types
  # Takes a hash of types. Validates each item in an hash using the type in the same position
  # in the list.
  class FixedHash < Base
    attr_reader :types

    def initialize(types)
      @types = types.each_with_object({}) {|(k, v), h| h[k] = T::Utils.coerce(v)}
    end

    # @override Base
    def name
      "{#{@types.map {|(k, v)| "#{k}: #{v}"}.join(', ')}}"
    end

    # @override Base
    def valid?(obj)
      return false unless obj.is_a?(Hash)

      @types.each do |key, type|
        return false unless type.valid?(obj[key])
      end

      obj.each_key do |key|
        return false unless @types[key]
      end

      true
    end

    # @override Base
    private def subtype_of_single?(other)
      case other
      when FixedHash
        # Using `subtype_of?` here instead of == would be unsound
        @types == other.types
      else
        false
      end
    end

    # This gives us better errors, e.g.:
    # "Expected {a: String}, got {a: TrueClass}"
    # instead of
    # "Expected {a: String}, got Hash".
    #
    # @override Base
    def describe_obj(obj)
      if obj.is_a?(Hash)
        "type {#{obj.map {|(k, v)| "#{k}: #{v.class}"}.join(', ')}}"
      else
        super
      end
    end
  end
end
