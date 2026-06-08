# frozen_string_literal: true
# typed: true

module T::Types
  class TypedHash < TypedEnumerable
    def underlying_class
      Hash
    end

    def initialize(keys:, values:)
      @inner_keys = keys
      @inner_values = values
    end

    # Technically we don't need this, but it is a nice api
    def keys
      @keys ||= T::Utils.coerce(@inner_keys)
    end

    # Technically we don't need this, but it is a nice api
    def values
      @values ||= T::Utils.coerce(@inner_values)
    end

    def type
      @type ||= T::Utils.coerce([keys, values])
    end

    # overrides Base
    def name
      "T::Hash[#{keys.name}, #{values.name}]"
    end

    # overrides Base
    #
    # Implemented directly (rather than `obj.is_a?(Hash) && super`) so the
    # per-call path checks against `keys`/`values` without re-deriving them
    # through TypedEnumerable's `case obj` + FixedArray pair plumbing.
    def recursively_valid?(obj)
      return false unless obj.is_a?(Hash)
      key_type = keys
      value_type = values
      obj.each_pair do |key, val|
        # Some objects (I'm looking at you Rack::Utils::HeaderHash) don't
        # iterate over a [key, value] array, so we can't just use the type.recursively_valid?(v)
        return false if !key_type.recursively_valid?(key) || !value_type.recursively_valid?(val)
      end
      true
    end

    # overrides Base
    def valid?(obj)
      obj.is_a?(Hash)
    end

    def new(...)
      Hash.new(...)
    end

    class Untyped < TypedHash
      def initialize
        super(keys: T.untyped, values: T.untyped)
      end

      def valid?(obj)
        obj.is_a?(Hash)
      end

      # overrides TypedHash
      #
      # Every entry trivially satisfies T.untyped, so the inherited O(n)
      # entry walk is pure overhead.
      def recursively_valid?(obj)
        obj.is_a?(Hash)
      end
    end
  end
end
