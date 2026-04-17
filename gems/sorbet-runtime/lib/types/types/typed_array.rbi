# typed: true

module T::Types
  class TypedArray < TypedEnumerable
    module Private
      module Pool
      end
    end

    class Untyped < TypedArray
      module Private
        INSTANCE = T.let(Untyped.new.freeze, Untyped)
      end
    end
  end
end
