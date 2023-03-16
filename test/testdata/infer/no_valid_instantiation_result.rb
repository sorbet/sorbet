# typed: true
extend T::Sig

sig do
  type_parameters(:U)
    .params(
      x: T.type_parameter(:U),
      f: T.proc.params(x: T.type_parameter(:U)).void,
    )
    .returns(T.type_parameter(:U))
end
def apply_f(x, f)
  x
end

sig {params(str: String).void}
def main(str)
  f = T.let(->(y) { y }, T.proc.params(x: Integer).void)
  res = apply_f(str, f)
  #     ^^^^^^^ error: Could not find valid instantiation of type parameters for `Object#apply_f`
  T.reveal_type(res) # error: `T.untyped`
end
