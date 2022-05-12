# typed: true

# Vaguely inspired by this post that dmitry linked me once:
# <https://lptk.github.io/programming/2019/09/13/type-projection.html>

module BadLower
  extend T::Generic

  A = type_member {{lower: String}}
end

module BadUpper
  extend T::Generic

  A = type_member {{upper: Integer}}
end

extend T::Sig

sig {
  params(x: T.all(BadLower, BadUpper)).void
  #               ^^^^^^^^ error: Generic class without type arguments `BadLower`
  #                         ^^^^^^^^ error: Generic class without type arguments `BadUpper`
}
def example1(x)
  T.reveal_type(x) # error: `T.all(BadLower[T.untyped], BadUpper[T.untyped])`
end

sig {
  type_parameters(:U)
  .params(x: T.all(BadLower[T.type_parameter(:U)], BadUpper[T.type_parameter(:U)])).void
  #                         ^^^^^^^^^^^^^^^^^^^^ error: `T.type_parameter(:<todo typeargument>)` is not a supertype of lower bound of type member `::BadLower::A`
  #                                                         ^^^^^^^^^^^^^^^^^^^^ error: `T.type_parameter(:<todo typeargument>)` is not a subtype of upper bound of type member `::BadUpper::A`
}
def example2(x)
  T.reveal_type(x) # error: `T.all(BadLower[T.untyped], BadUpper[T.untyped])`
end

sig {
  type_parameters(:U)
  # here, x has bad bounds and no error, but maybe that's fine because
  # String <: T.untyped <: Integer is just natural due to untyped
  # (i.e., maybe we should only do bad bounds checking for non-untyped)
  .params(x: T.all(BadLower[T.untyped], BadUpper[T.untyped])).void
}
def example3(x)
  T.reveal_type(x) # error: `T.all(BadLower[T.untyped], BadUpper[T.untyped])`
end
