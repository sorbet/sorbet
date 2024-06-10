# typed: true
extend T::Sig

# sig do
#   type_parameters(:U)
#     .params(x: T.type_parameter(:U))
#     .void
# end
# def example(x = 0)
# end





sig do
  type_parameters(:U)
    .params(x: T.type_parameter(:U))
    .void
end
def example2(x)
  T.let(x, Integer)
end
