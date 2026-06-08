# frozen_string_literal: true
# typed: true

module T::Types
  # Validates that an object is equal to another T::Enum singleton value.
  class TEnum < Base
    attr_reader :val

    def initialize(val)
      @val = val
    end

    def build_type
      nil
    end

    # overrides Base
    def name
      # Strips the #<...> off, just leaving the ...
      # Reasoning: the user will have written something like
      #   T.any(MyEnum::A, MyEnum::B)
      # in the type, so we should print what they wrote in errors, not:
      #   T.any(#<MyEnum::A>, #<MyEnum::B>)
      @val.inspect[2..-2]
    end

    # overrides Base
    def valid?(obj)
      @val == obj
    end

    # overrides Base
    private def subtype_of_single?(other)
      case other
      when TEnum
        @val == other.val
      when Simple
        other.raw_type.===(@val)
      else
        false
      end
    end

    module Private
      module Pool
        # Enum values are frozen singletons (bound in `enums do`), so they
        # fully determine a TEnum and make perfect weak keys: inline call
        # sites like `T.cast(x, MyEnum::A)` stop allocating a fresh TEnum
        # per evaluation. Weak-keyed and weak-valued (the enum class retains
        # its values via @values, so entries live as long as the class);
        # GC-dropped entries transparently re-derive, racy writes are benign
        # double-inits, as in Simple::Private::Pool.
        @cache = ObjectSpace::WeakMap.new

        def self.type_for_enum(val)
          cached = @cache[val]
          return cached if cached

          @cache[val] = TEnum.new(val)
        end
      end
    end
  end
end
