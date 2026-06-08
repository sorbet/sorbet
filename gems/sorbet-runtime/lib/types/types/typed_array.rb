# frozen_string_literal: true
# typed: true

module T::Types
  class TypedArray < TypedEnumerable
    # overrides Base
    def name
      "T::Array[#{type.name}]"
    end

    def underlying_class
      Array
    end

    # overrides Base
    #
    # Implemented directly (rather than `obj.is_a?(Array) && super`) so the
    # per-call path skips the super dispatch and TypedEnumerable's
    # `is_a?(Enumerable)` + `case obj` re-classification.
    def recursively_valid?(obj)
      return false unless obj.is_a?(Array)
      type_ = self.type
      len = obj.length
      i = 0
      while i < len
        return false unless type_.recursively_valid?(obj[i])
        i += 1
      end
      true
    end

    # overrides Base
    def valid?(obj)
      obj.is_a?(Array)
    end

    def new(...)
      Array.new(...)
    end

    module Private
      module Pool
        CACHE_FROZEN_OBJECTS = begin
          ObjectSpace::WeakMap.new[1] = 1
          true # Ruby 2.7 and newer
                               rescue ArgumentError # Ruby 2.6 and older
                                 false
        end

        @cache = ObjectSpace::WeakMap.new

        def self.type_for_module(mod)
          cached = @cache[mod]
          return cached if cached

          type = TypedArray.new(mod)

          if CACHE_FROZEN_OBJECTS || (!mod.frozen? && !type.frozen?)
            @cache[mod] = type
          end
          type
        end
      end
    end

    class Untyped < TypedArray
      def initialize
        super(T::Types::Untyped::Private::INSTANCE)
      end

      def valid?(obj)
        obj.is_a?(Array)
      end

      # overrides TypedArray
      #
      # Every element trivially satisfies T.untyped, so the inherited O(n)
      # element walk is pure overhead.
      def recursively_valid?(obj)
        obj.is_a?(Array)
      end

      def freeze
        build_type # force lazy initialization before freezing the object
        super
      end

      module Private
        INSTANCE = Untyped.new.freeze
      end
    end
  end
end
