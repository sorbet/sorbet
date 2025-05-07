# typed: strict
# enable-experimental-rbs-signatures: true
# enable-experimental-rbs-assertions: true

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
