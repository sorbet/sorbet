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

  sig {params(blk: T.proc.void).void}
  def test_block_type_specified(&blk); end

  sig {void}
  def test_block_type_not_specified(&blk); end
                                   # ^^^ error: Malformed `sig`. Type not specified for argument `blk`

  sig {void}
  def test_yield
    yield
  end

  sig {void}
  def test_block_type_not_specified_later; end
end

class A
  def test_block_type_not_specified_later(&my_custom_block_name); end
                                         # ^^^^^^^^^^^^^^^^^^^^ error: Malformed `sig`. Type not specified for argument `my_custom_block_name`
end