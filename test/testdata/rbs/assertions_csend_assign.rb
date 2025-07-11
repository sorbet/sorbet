# typed: strict
# enable-experimental-rbs-comments: true

# let

class Let
  #: (*untyped) -> void
  def foo=(*args); end

  #: -> untyped
  def foo; end
end

let1 = Let.new #: Let?
let1&.foo = "foo" #: String

let2 = Let.new #: Let?
let2&.foo&.foo = "foo" #: String

let3 = Let.new #: Let?
let3&.foo&.foo = "foo", "bar" #: Array[String]
