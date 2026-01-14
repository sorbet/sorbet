# typed: true

class A
  extend T::Sig

  sig {params(foo: Integer).void}
  def test_arg_type_specified(foo); end

  sig {void}
  def test_arg_type_not_specified(foo); end
                                # ^^^ error: Malformed `sig`. Type not specified for parameter `foo`

  sig {params(foo: Integer).void}
  def test_kwarg_type_specified(foo:); end

  sig {void}
  def test_kwarg_type_not_specified(foo:); end
                                  # ^^^^ error: Malformed `sig`. Type not specified for parameter `foo`

  sig {params(blk: T.proc.void).void}
  def test_block_type_specified(&blk); end

  sig {void}
  def test_block_type_not_specified(&blk); end
                                   # ^^^ error: Malformed `sig`. Type not specified for parameter `blk`

  sig {void}
  def test_yield # error: does not mention a block parameter
    yield
  # ^^^^^ error: Method `call` does not exist on `NilClass`
  end

  sig {void}
  def test_block_type_not_specified_later; end
# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: Malformed `sig`. Type not specified for parameter `my_custom_block_name`

  sig { params(x: Integer,).returns(Integer) }
  def calls_yield_with_params(x) # error: does not mention a block parameter
    yield
  # ^^^^^ error: Method `call` does not exist on `NilClass`
  end

  sig { params(x: Integer,).returns(Integer) }
  def calls_yield_with_params_default(x: 0) # error: does not mention a block parameter
    yield
  # ^^^^^ error: Method `call` does not exist on `NilClass`
  end
end

class A
  def test_block_type_not_specified_later(&my_custom_block_name); end
end
