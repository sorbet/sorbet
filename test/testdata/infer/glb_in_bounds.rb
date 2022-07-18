# typed: true
extend T::Sig

class Parent
  extend T::Generic
  A = type_member
end

class Child1 < Parent
  A = type_member {{upper: Integer}}
end

class Child2 < Parent
  A = type_member {{lower: Integer}}
end

sig {
  params(
    x: T.all(
      Parent[String],
      Child1[T.untyped]
    ),
    y: T.any(
      Parent[String],
      Child1[T.untyped]
    ),
  ).void
}
def example1(x, y)
  T.reveal_type(x) # error: `Child1[T.untyped]`
  T.reveal_type(y) # error: `Parent[T.untyped]`
end

sig {
  params(
    x: T.all(
      Parent[String],
      Child2[T.untyped]
    ),
    y: T.any(
      Parent[String],
      Child2[T.untyped]
    ),
  ).void
}
def example2(x, y)
  T.reveal_type(x) # error: `Child2[T.untyped]`
  T.reveal_type(y) # error: `Parent[T.untyped]`
end
