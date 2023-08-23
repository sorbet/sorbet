# typed: true
class Module; include T::Sig; end

# Basically the only thing you can do with argument forwarding is to make
# everything untyped.
#
# What you might want Sorbet to do is to infer the signature of the thing
# you're trying to forward arguments to, but sorbet doesn't let the types of
# one method depend on the types of another method, for performance.

class OnlyFwdArgs
  sig {params("...": T.untyped).void}
  def self.example1(...)
    p(...)
  # ^^^^^^ error: Splats are only supported where the size of the array is known statically

    T.unsafe(self).p(...)
  end

  sig do
    type_parameters(:U)
      .params("...": T.type_parameter(:U))
    #         ^^^^^^ error: Block argument type must be either `Proc` or a `T.proc` type
      .void
  end
  def self.example2(...)
  end

  sig do
    params("...": T.proc.void).void
  end
  def self.example3(...)
  end

  example3(0)
  #        ^ error: Expected `T.proc.void` but found `Integer(0)` for argument `"...":`
  #          ^ error: requires a block parameter, but no block was passed

  sig do
    params("...": T.nilable(T.proc.void)).void
  end
  def self.example4(...)
  end

  example4(0)
  #        ^ error: Expected `T.nilable(T.proc.void)` but found `Integer(0)` for argument `"...":`
end


class WithLeadingArgs
  sig {params(x: Integer, "...": T.untyped).void}
  def self.example1(x, ...)
    T.reveal_type(x) # error: `Integer`

    p(...)
  # ^^^^^^ error: Splats are only supported where the size of the array is known statically

    T.unsafe(self).p(...)
  end

  example1('')
  #        ^^ error: Expected `Integer` but found `String("")` for argument `x`
  example1('', 0, 'anything else', foo: :bar)
  #        ^^ error: Expected `Integer` but found `String("")` for argument `x`

  sig do
    type_parameters(:U)
      .params(
        x: Integer,
        "...": T.type_parameter(:U)
      # ^^^^^^ error: Block argument type must be either `Proc` or a `T.proc` type
      )
      .void
  end
  def self.example2(x, ...)
    T.reveal_type(x) # error: `Integer`
  end

  sig do
    params(x: Integer, "...": T.proc.void).void
  end
  def self.example3(x, ...)
  end

  example3(0)
  #          ^ error: requires a block parameter, but no block was passed

  sig do
    params(x: Integer, "...": T.nilable(T.proc.void)).void
  end
  def self.example4(x, ...)
  end

  example4(0)
end

