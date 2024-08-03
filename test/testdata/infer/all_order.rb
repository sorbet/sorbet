# typed: true

extend T::Sig

class Parent
  extend T::Generic
  Elem = type_member(:out)
end

class Child < Parent
  extend T::Generic
  Elem = type_member(:out)
end

module Other1; end
module Other2; end

sig {
  params(
    x1: T.all(Other2, Parent[Other1], Child[T.anything]),
    x2: T.all(Parent[Other1], Other2, Child[T.anything]),
    x3: T.all(Parent[Other1], Child[T.anything], Other2),
  ).void
}
def example(x1, x2, x3)
  T.reveal_type(x1) # error: `T.all(Child[Other1], Other2)`
  takes_mchild_other1(x1)
  T.reveal_type(x2) # error: `T.all(Child[Other1], Other2)`
  takes_mchild_other1(x2)
  T.reveal_type(x3) # error: `T.all(Child[Other1], Other2)`
  takes_mchild_other1(x3)
end

sig { params(mchild_other1: Child[Other1]).void }
def takes_mchild_other1(mchild_other1)
end
