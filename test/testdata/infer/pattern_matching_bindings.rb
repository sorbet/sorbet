# typed: true
extend T::Sig

class Box
  extend T::Sig
  extend T::Generic
  Value = type_member

  sig { params(value: Value).void }
  def initialize(value)
    @value = value
  end

  sig { returns([Value]) }
  def deconstruct = [@value]

  sig { params(keys: T.nilable(T::Array[Symbol])).returns({value: Value}) }
  def deconstruct_keys(keys) = {value: @value}
end

class Opaque; end

Point = Data.define(:x, :y)

sig { params(box: Box[Integer]).void }
def const_array_pattern(box)
  case box
  in Box(value)
    T.reveal_type(value) # error: Revealed type: `Integer`
  end
end

sig { params(box: Box[Integer]).void }
def const_hash_pattern(box)
  case box
  in Box(value:)
    T.reveal_type(value) # error: Revealed type: `Integer`
  end
end

sig { params(x: T.any(Box[Integer], Box[String])).void }
def union_subject(x)
  case x
  in Box(value)
    T.reveal_type(value) # error: Revealed type: `T.any(Integer, String)`
  end
end

sig { params(x: T.any(Integer, String, T::Array[Symbol])).void }
def value_and_capture(x)
  case x
  in Integer => i
    T.reveal_type(i) # error: Revealed type: `Integer`
  in String => s if s.empty?
    T.reveal_type(s) # error: Revealed type: `String`
  in [Symbol => first, *rest]
    T.reveal_type(first) # error: Revealed type: `Symbol`
    T.reveal_type(rest) # error: Revealed type: `T::Array[Symbol]`
  in other
    T.reveal_type(other) # error: Revealed type: `T.any(T::Array[Symbol], String)`
  end
end

sig { params(pair: [Integer, String]).void }
def tuple_pattern(pair)
  case pair
  in [a, b]
    T.reveal_type(a) # error: Revealed type: `Integer`
    T.reveal_type(b) # error: Revealed type: `String`
  end

  pair => [*, last]
  T.reveal_type(last) # error: Revealed type: `String`
end

sig { params(shape: {name: String, age: Integer}).void }
def shape_pattern(shape)
  case shape
  in {name:, age: Integer => age}
    T.reveal_type(name) # error: Revealed type: `String`
    T.reveal_type(age) # error: Revealed type: `Integer`
  end
end

sig { params(hash: T::Hash[Symbol, Float]).void }
def hash_pattern(hash)
  if hash in {ratio:}
    T.reveal_type(ratio) # error: Revealed type: `Float`
  end
end

sig { params(point: Point).void }
def untyped_deconstruct(point)
  case point
  in Point(x, y)
    T.reveal_type(x) # error: Revealed type: `T.untyped`
  end
end

sig { params(x: Opaque).void }
def no_deconstruct(x)
  case x
  in [a]
    T.reveal_type(a) # error: Revealed type: `T.untyped`
  end
end

sig { params(x: T.untyped).void }
def find_pattern(x)
  case x
  in [*, Integer => found, *]
    T.reveal_type(found) # error: Revealed type: `T.untyped`
  end
end

sig { params(x: T.any(Integer, String)).returns(Integer) }
def no_else_raises(x)
  case x
  in Integer => i then i
  in String => s then s.length
  end
end

sig { params(expected: Integer, x: Integer).void }
def pinned(expected, x)
  case x
  in ^expected
    T.reveal_type(x) # error: Revealed type: `Integer`
  end
end
