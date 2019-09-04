# typed: true
class Foo
  extend(T::Sig)
  
  sig { returns(Array) }
  def array_method
    [1, "2", :foo]
  end

  sig { returns(Hash) }
  def hash_method
    { a: 1, b: "2", "c" => :foo }
  end

  sig { returns(Set) }
  def set_method
    Set.new([1, "2", :foo])
  end

  sig { returns(Range) }
  def range_method
    'a'..'b'
  end

  sig { returns(Enumerable) }
  def enumerable_method
    [1, "2", :foo].to_enum
  end

  sig { returns(Enumerator) }
  def enumerator_method
    [1, "2", :foo].to_enum
  end

  sig { returns(Array) }
  def bad_array_method
    1 # error: Returning value that does not conform to method result type
  end

  sig { returns(Hash) }
  def bad_hash_method
    1 # error: Returning value that does not conform to method result type
  end

  sig { returns(Set) }
  def bad_set_method
    1 # error: Returning value that does not conform to method result type
  end

  sig { returns(Range) }
  def bad_range_method
    1 # error: Returning value that does not conform to method result type
  end

  sig { returns(Enumerable) }
  def bad_enumerable_method
    1 # error: Returning value that does not conform to method result type
  end

  sig { returns(Enumerator) }
  def bad_enumerator_method
    1 # error: Returning value that does not conform to method result type
  end
end

T.reveal_type(Foo.new.array_method) # error: Revealed type: `T::Array[T.untyped]`
T.reveal_type(Foo.new.hash_method) # error: Revealed type: `T::Hash[T.untyped, T.untyped]`
T.reveal_type(Foo.new.set_method) # error: Revealed type: `T::Set[T.untyped]`
T.reveal_type(Foo.new.range_method) # error: Revealed type: `T::Range[T.untyped]`
T.reveal_type(Foo.new.enumerable_method) # error: Revealed type: `T::Enumerable[T.untyped]`
T.reveal_type(Foo.new.enumerator_method) # error: Revealed type: `T::Enumerator[T.untyped]`
