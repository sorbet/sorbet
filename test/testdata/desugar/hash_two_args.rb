# typed: true
extend T::Helpers

sig {params(foo: T::Hash[Symbol, Integer]).void}
def test_enumeration(foo)
    foo.each do |k, v|
        T.assert_type!(k, Symbol)
        T.assert_type!(v, Integer)
    end
    foo.map do |k, v|
        T.assert_type!(k, Symbol)
        T.assert_type!(v, Integer)
    end

    foo.each do |(k, v)|
        T.assert_type!(k, Symbol)
        T.assert_type!(v, Integer)
    end
    foo.map do |(k, v)|
        T.assert_type!(k, Symbol)
        T.assert_type!(v, Integer)
    end

    foo.each do |kv|
        T.assert_type!(kv, [Symbol, Integer])
    end
    foo.map do |kv|
        T.assert_type!(kv, [Symbol, Integer])
    end
end

test_enumeration({a: 1})
