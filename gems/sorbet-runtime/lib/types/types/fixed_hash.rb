# frozen_string_literal: true
# typed: true

module T::Types
  # Takes a hash of types. Validates each item in a hash using the type in the same position
  # in the list.
  class FixedHash < Base
    attr_reader :types

    def initialize(types)
      @types = types.transform_values {|v| T::Utils.coerce(v)}
    end

    # @override Base
    def name
      entries = @types.map do |(k, v)|
        if Symbol === k && ":#{k}" == k.inspect
          "#{k}: #{v}"
        else
          "#{k.inspect} => #{v}"
        end
      end

      "{#{entries.join(', ')}}"
    end

    # @override Base
    def recursively_valid?(obj)
      return false unless obj.is_a?(Hash)
      return false if @types.any? {|key, type| !type.recursively_valid?(obj[key])}
      return false if obj.any? {|key, _| !@types[key]}
      true
    end

    # @override Base
    def valid?(obj)
      return false unless obj.is_a?(Hash)
      return false if @types.any? {|key, type| !type.valid?(obj[key])}
      return false if obj.any? {|key, _| !@types[key]}
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
