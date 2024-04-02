# typed: true

extend T::Sig

class Upper
end

class Middle < Upper
end

class Lower < Middle
end

class A
  extend T::Generic
  X = type_member
  Y = type_member(:in)
  Z = type_member(:out)
end

T.let(A[String, Lower, Upper].new, A[Middle, Middle, Middle])

class Box1
  extend T::Generic
  Elem = type_member
end

class Box2
  extend T::Generic
  Elem = type_member
end

sig { params(box1: Box1[Integer]).void }
def takes_box1(box1)
  T.let(box1, Box2[Integer])
  T.let(box1, Integer)
end

class Box
  extend T::Generic
  Elem = type_member
end

sig { params(box: Box[T.any(Integer, String)]).void }
def takes_box_int_str(box); end

sig { params(box: Box[Integer]).void }
def takes_box_integer(box)
  takes_box_int_str(box)
end
