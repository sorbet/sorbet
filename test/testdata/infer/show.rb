# typed: true

class Box
    extend T::Generic
    Elem = type_member
end

module A
end
module B
end

class Main
    extend T::Helpers

    sig(a: Integer).returns(NilClass)
    def int(a)
        nil
    end

    def main
        int("string") # error: `String("string")` doesn't match `Integer` for argument `a`

        foo = Object.new
        # T.assert_type!(foo, 3) # commented-out-error: 3
        T.assert_type!(foo, Integer) # error: asserted type `Integer`
        T.assert_type!(foo, Array) # error: asserted type `T::Array[T.untyped]`
        T.assert_type!(foo, T.any(String, Integer)) # error: asserted type `T.any(String, Integer)`
        T.assert_type!(foo, T.all(A, B)) # error: asserted type `T.all(A, B)`
        T.assert_type!(foo, T.nilable(String)) # error: asserted type `T.nilable(String)`
        T.assert_type!(foo, T.any(String, NilClass)) # error: asserted type `T.nilable(String)`
        T.assert_type!(foo, T.any(NilClass, String)) # error: asserted type `T.nilable(String)`
        T.assert_type!(foo, T.any(String, Symbol, NilClass)) # error: asserted type `T.nilable(T.any(String, Symbol))`
        T.assert_type!(foo, T.any(String, Symbol, NilClass, Integer, Float)) # error: asserted type `T.any(String, Symbol, NilClass, Integer, Float)`
        T.assert_type!(foo, T::Array[Integer]) # error: asserted type `T::Array[Integer]`
        T.assert_type!(foo, T::Hash[T.any(Symbol, String), Integer]) # error: asserted type `T::Hash[T.any(Symbol, String), Integer]`
        T.assert_type!(foo, Box[Integer]) # error: asserted type `Box[Integer]`
        T.assert_type!(foo, Box[Box[Integer]]) # error: asserted type `Box[Box[Integer]]`
        T.assert_type!(foo, [Integer, String, Symbol]) # error: asserted type `[Integer, String, Symbol]`
        # T.assert_type!(foo, {a: Integer, "b" => String}) # commented-out-error: asserted type `{a: Integer, "b" => String}`
    end
end
