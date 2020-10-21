# frozen_string_literal: true
# typed: true

module T::Types
  class TypedEnumerator < TypedEnumerable
    attr_reader :type

    def underlying_class
      Enumerator
    end

    # @override Base
    def name
      "T::Enumerator[#{@type.name}]"
    end

    # @override Base
    def recursively_valid?(obj)
      obj.is_a?(Enumerator) && super
    end

    # @override Base
    def valid?(obj)
      obj.is_a?(Enumerator)
    end

    def new(*args, &blk)
      T.unsafe(Enumerator).new(*args, &blk)
    end

    class Untyped < TypedEnumerator
      def initialize
        super(T.untyped)
      end

      def valid?(obj)
        obj.is_a?(Enumerator)
      end
    end
  end
end
