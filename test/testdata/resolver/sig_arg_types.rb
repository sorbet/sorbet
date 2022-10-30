# typed: true

class A
  extend T::Sig

  sig {params(foo: Integer).void}
  def test_arg_type_specified(foo); end

  sig {void}
  def test_arg_type_not_specified(foo); end
                                # ^^^ error: Malformed `sig`. Type not specified for argument `foo`

  sig {params(foo: Integer).void}
  def test_kwarg_type_specified(foo:); end

  sig {void}
  def test_kwarg_type_not_specified(foo:); end
                                  # ^^^^ error: Malformed `sig`. Type not specified for argument `foo`
end
