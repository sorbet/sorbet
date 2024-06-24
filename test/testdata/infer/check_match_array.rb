# typed: true

TYPES = T.let(
  [Integer, String],
  [T.class_of(Integer), T.class_of(String)]
)

def foo(x)
  case x
  when *[Integer, String]
    T.reveal_type(x) # error: `T.untyped`
  end

  case x
  when *TYPES
    T.reveal_type(x) # error: `T.untyped`
  end
end
