# typed: true

class A
  extend T::Sig

  class WhateverAReturnsFromToHash ; end

  sig {returns(WhateverAReturnsFromToHash)}
  def to_hash 
    WhateverAReturnsFromToHash.new
  end
end

foo = {x: "hello", y: 333, z: A.new}
T.reveal_type(foo) # error: Revealed type: `{x: String("hello"), y: Integer(333), z: A} (shape of T::Hash[Symbol, T.any(String, Integer, A)])`
T.reveal_type(foo.to_hash) # error: Revealed type: `{x: String("hello"), y: Integer(333), z: A} (shape of T::Hash[Symbol, T.any(String, Integer, A)])`

bar = {x: "yolo", y: 209, z: "hello again"}

if T.unsafe(false)
  baz = foo
else
  baz = bar
end

T.reveal_type(baz) # error: Revealed type: `{x: String, y: Integer, z: T.any(A, String)} (shape of T::Hash[Symbol, T.any(A, String, Integer)])`
T.reveal_type(baz.to_hash) # error: Revealed type: `{x: String, y: Integer, z: T.any(A, String)} (shape of T::Hash[Symbol, T.any(A, String, Integer)])`

if T.unsafe(false)
  qux = baz
else
  qux = A.new
end

T.reveal_type(qux) # error: Revealed type: `T.any(A, {x: String, y: Integer, z: T.any(A, String)})`
T.reveal_type(qux.to_hash) # error: Revealed type: `T.any(A::WhateverAReturnsFromToHash, {x: String, y: Integer, z: T.any(A, String)})`
