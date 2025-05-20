# typed: strict
# enable-experimental-rbs-comments: true

# let

class Let
  #: (*untyped) -> void
  def foo=(*args); end

  #: -> untyped
  def foo; end
end

let1 = Let.new
let1.foo = "foo" #: String
T.reveal_type(let1.foo) # error: Revealed type: `T.untyped`

let2 = Let.new
let2.foo.foo = "foo" #: String
T.reveal_type(let2.foo) # error: Revealed type: `T.untyped`

let3 = Let.new
let3.foo.foo = "foo", "bar" #: Array[String]
T.reveal_type(let3.foo) # error: Revealed type: `T.untyped`

class BracketsAssign
  #: (*untyped) -> void
  def []=(*args); end
end

brackets_assign = BracketsAssign.new

brackets_assign[] = "foo" #: as untyped
  .unexisting_method

brackets_assign[:a] = "bar" #: as untyped
  .unexisting_method

brackets_assign[:a, :b] = "baz" #: as untyped
  .unexisting_method

brackets_assign[:a, :b, :c] = "qux" #: as untyped
  .unexisting_method

self #: as untyped
  .unexisting_method = 42
