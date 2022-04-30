# frozen_string_literal: true
# typed: true

module T::Types
  class TypedEnumeratorYielder < TypedEnumerable
    attr_reader :type

    def underlying_class
      Enumerator::Yielder
    end

    # overrides Base
    def name
      "T::Enumerator::Yielder[#{@type.name}]"
    end

    # overrides Base
    def recursively_valid?(obj)
      obj.is_a?(Enumerator::Yielder) && super
    end

    # overrides Base
    def valid?(obj)
      obj.is_a?(Enumerator::Yielder)
    end

    def new(*args, &blk)
      T.unsafe(Enumerator::Yielder).new(*args, &blk)
    end

    class Untyped < TypedEnumeratorYielder
      def initialize
        super(T.untyped)
      end

      def valid?(obj)
        obj.is_a?(Enumerator::Yielder)
      end
    end
  end
end
