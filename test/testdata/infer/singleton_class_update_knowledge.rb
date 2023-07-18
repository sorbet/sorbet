# typed: true
extend T::Sig

class A; extend T::Helpers; final!; end
class B; extend T::Helpers; final!; end

sig {params(x: T.any(T.class_of(A), T.class_of(B))).void}
def test1(x)
  if x == A
    T.reveal_type(x) # error: `T.class_of(A)`
  else
    T.reveal_type(x) # error: `T.class_of(B)`
  end
end

sig {params(x: T.any(T.class_of(A), T.class_of(B))).void}
def test2(x)
  if A == x
    T.reveal_type(x) # error: `T.class_of(A)`
  else
    T.reveal_type(x) # error: `T.class_of(B)`
  end
end

sig {params(x: T.any(T::Array[Integer], T::Set[String])).void}
def test3(x)
  if x == []
    T.reveal_type(x) # error: `[] (0-tuple)`
  else
    T.reveal_type(x) # error: `T.any(T::Array[Integer], T::Set[String])`
  end
end

sig {params(x: T.any(T::Array[Integer], T::Set[String])).void}
def test4(x)
  if [] == x
    T.reveal_type(x) # error: `[] (0-tuple)`
  else
    T.reveal_type(x) # error: `T.any(T::Array[Integer], T::Set[String])`
  end
end

class C
  extend T::Generic
  Elem = type_template
  final!
end

class D
  extend T::Generic
  Elem = type_template
  final!
end

sig {params(x: T.any(T.class_of(C), T.class_of(D))).void}
def test5(x)
  if x == C
    T.reveal_type(x) # error: `T.class_of(C)[C, T.untyped]`
  else
    T.reveal_type(x) # error: `T.class_of(D)[D, T.untyped]`
  end
end

sig {params(x: T.any(T.class_of(C), T.class_of(D))).void}
def test6(x)
  if C == x
    T.reveal_type(x) # error: `T.class_of(C)[C, T.untyped]`
  else
    T.reveal_type(x) # error: `T.class_of(D)[D, T.untyped]`
  end
end

module E; extend T::Helpers; final!; end
module F; extend T::Helpers; final!; end

sig {params(x: T.any(T.class_of(E), T.class_of(F))).void}
def test7(x)
  if x == E
    T.reveal_type(x) # error: `T.class_of(E)`
  else
    T.reveal_type(x) # error: `T.class_of(F)`
  end
end

sig {params(x: T.any(T.class_of(E), T.class_of(F))).void}
def test8(x)
  if E == x
    T.reveal_type(x) # error: `T.class_of(E)`
  else
    T.reveal_type(x) # error: `T.class_of(F)`
  end
end

class Parent; end
class Child < Parent; end

sig {params(x: T.class_of(Parent)).void}
def test9(x)
  if x == Parent
    T.reveal_type(x) # error: `T.class_of(Parent)`
  else
    # `x` might be `Child`
    T.reveal_type(x) # error: `T.class_of(Parent)`
  end
end

sig {params(x: T.class_of(Parent)).void}
def test10(x)
  if Parent == x
    T.reveal_type(x) # error: `T.class_of(Parent)`
  else
    # `x` might be `Child`
    T.reveal_type(x) # error: `T.class_of(Parent)`
  end
end
