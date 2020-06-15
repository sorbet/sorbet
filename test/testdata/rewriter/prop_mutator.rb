# typed: true
class Foo
    include T::Props
    prop :int, Integer
    prop :hash, Hash
    prop :t_hash, T::Hash[T.untyped, T.untyped]
    prop :t_hash_type, T::Hash[Symbol, String]
    prop :array, Array
    prop :t_array, T::Array[T.untyped]
    prop :t_array_type, T::Array[String]
    prop :doc, Foo
end

# Could work if we could figure out this was a class
Foo::Mutator.new.int = 3 # error: Method `int=` does not exist on `Foo::Mutator`
Foo::Mutator.new.hash[:a] = :b
Foo::Mutator.new.t_hash[:a] = :b
Foo::Mutator.new.t_hash['a'] = :b
Foo::Mutator.new.t_hash_type[:a] = 'b'
Foo::Mutator.new.t_hash_type['a'] = 'b' # error: Expected `Symbol` but found `String("a")` for argument `key`
Foo::Mutator.new.t_hash_type[:a] = :b # error: Expected `String` but found `Symbol(:"b")` for argument `value`
Foo::Mutator.new.array << :a
Foo::Mutator.new.t_array << 'a'
Foo::Mutator.new.t_array << :a
Foo::Mutator.new.t_array_type << 'a'
Foo::Mutator.new.t_array_type << :a # error: Expected `String` but found `Symbol(:"a")` for argument `value`
# Could work if we could figure out to return a `Foo::Mutator`
Foo::Mutator.new.doc.int = 3 # error: Method `doc` does not exist on `Foo::Mutator`
