# typed: true
extend T::Sig

class A; end
class B; end

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
end

class D
  extend T::Generic
  Elem = type_template
end

sig {params(x: T.any(T.class_of(C), T.class_of(D))).void}
def test5(x)
  if x == C
    T.reveal_type(x) # error: `T.class_of(C)[T.untyped]`
  else
    T.reveal_type(x) # error: `T.class_of(D)[T.untyped]`
  end
end

sig {params(x: T.any(T.class_of(C), T.class_of(D))).void}
def test6(x)
  if C == x
    T.reveal_type(x) # error: `T.class_of(C)[T.untyped]`
  else
    T.reveal_type(x) # error: `T.class_of(D)[T.untyped]`
  end
end

module E; end
module F; end

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

