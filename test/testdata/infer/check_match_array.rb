# typed: true

TYPES = T.let(
  [Integer, String],
  [T.class_of(Integer), T.class_of(String)]
)

def foo(x)
  case x
  when *[Integer, String]
    T.reveal_type(x) # error: `T.any(Integer, String)`
  end

  case x
  when *TYPES
    T.reveal_type(x) # error: `T.any(Integer, String)`
  end

  case x
  when *[]
    T.reveal_type(x) # error: This code is unreachable
  end

  case x
  when *[Integer]
    T.reveal_type(x) # error: `Integer`
  end
end
