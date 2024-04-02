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

