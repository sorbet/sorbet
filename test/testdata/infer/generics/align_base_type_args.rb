# typed: true
extend T::Sig

module A
  extend T::Generic
  X = type_member
end

module B
  extend T::Generic
  Y = type_member
end

class AB
  extend T::Generic

  include A
  include B
  X = type_member
  Y = type_member
end

sig {params(ab: T.all(AB[T.untyped, T.untyped], A[Integer], B[String])).void}
def example1(ab)
  T.reveal_type(ab) # error: `AB[Integer, String]`
end

sig {params(ba: T.all(AB[T.untyped, T.untyped], B[Float], A[Symbol])).void}
def example2(ba)
  T.reveal_type(ba) # error: `AB[Symbol, Float]`
end
