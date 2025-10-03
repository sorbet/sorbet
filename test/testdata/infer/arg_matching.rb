# typed: true
class TestArgs
  extend T::Sig

  def any; end

  sig {returns(T::Hash[Integer, Integer])}
  def a_hash; end # error: Expected `T::Hash[Integer, Integer]` but found `NilClass` for method result type

  def required(a, b)
  end

  def call_required
    required(1)
    #         ^ error: Not enough arguments
    required(1, 2)
    required(1, 2, 3)
    #              ^ error: Expected: `2`, got: `3`

    required 1
    #         ^ error: Not enough arguments
    required 1, 2
    required 1, 2, 3
    #              ^ error: Too many arguments

    required
    #       ^ error: Not enough arguments
    required {}
    #       ^ error: Not enough arguments
    required()
    #        ^ error: Not enough arguments
    required( )
    #         ^ error: Not enough arguments
    required() {}
    #        ^ error: Not enough arguments
    required(0) {}
    #         ^ error: Not enough arguments
    required(0, )
    #         ^ error: Not enough arguments
    required(any:)
    #            ^ error: Not enough arguments
    required(any:,)
    #            ^ error: Not enough arguments
    required((0))
    #          ^ error: Not enough arguments
  end

  def optional(a, b=1)
  end

  def call_optional
    optional(1)
    optional(1, 2)
    optional(1, 2, 3)
    #              ^ error: Expected: `1..2`, got: `3`
  end

  sig do
    params(
      a: Integer,
      b: Integer,
    )
    .returns(NilClass)
  end
  def kwarg(a, b:)
  end

  def call_kwarg
    # "too many arguments" and "missing keyword argument b"
    kwarg(1, 2)
    #        ^ error: Too many positional arguments provided for method `TestArgs#kwarg`. Expected: `1`, got: `2`
    #         ^ error: Missing required keyword argument `b` for method `TestArgs#kwarg`
    kwarg(1)
    #      ^ error: Missing required keyword argument `b`

    kwarg(1, b: 2)
    kwarg(1, b: 2, c: 3)
    #              ^^^^ error: Unrecognized keyword argument `c`
    kwarg(1, {})
    #          ^ error: Missing required keyword argument `b`
    kwarg(1, b: "hi")
    #           ^^^^ error: Expected `Integer` but found `String("hi")` for argument `b`
    kwarg(1, any)
    kwarg(1, a_hash)
    #        ^^^^^^ error: Cannot call `TestArgs#kwarg` with a `Hash` keyword splat because the method has required keyword parameters
  end

  sig do
    params(
      x: Integer
    )
    .returns(NilClass)
  end
  def repeated(*x)
  end

  def call_repeated
    repeated
    repeated(1, 2, 3)
    repeated(1, "hi")
    #           ^^^^ error: Expected `Integer` but found `String("hi")` for argument `x`

    # We error on each incorrect argument
    repeated("hi", "there")
    #        ^^^^ error: Expected `Integer` but found `String("hi")` for argument `x`
    #              ^^^^^^^ error: Expected `Integer` but found `String("there")` for argument `x`
  end

  sig do
    params(
      x: Integer,
      y: Integer,
      z: T::Hash[Integer, Integer],
      w: String,
      u: Integer,
      v: Integer
    )
    .returns(NilClass)
  end
  def mixed(x, y=T.unsafe(nil), z=T.unsafe(nil), *w, u:, v: 0)
  end

  def call_mixed
    mixed(0, u: 1)
    mixed(0, 1, u: 1)
    mixed(0, 1, {z: 1}, u: 1)
    mixed(0, 1, {z: 1}, "hi", "there", u: 1, v: 0)
  end

  def optkw(x, y=T.unsafe(nil), u:)
  end

  def call_optkw
    # There's ambiguity here about whether to report `u` or `x` as
    # missing; We follow Ruby in complaining about `u`.
    optkw(u: 1)
    #         ^ error: Missing required keyword argument `u`
    optkw(1, 2, 3)
    #            ^    error: Missing required keyword argument `u` for method `TestArgs#optkw`
    #           ^ error: Too many positional arguments provided for method `TestArgs#optkw`. Expected: `1..2`, got: `3`
  end

  def requires_mixed(a, b, c:); end

  def call_requires_three
    requires_mixed 1
    #               ^ error: Not enough arguments
    #               ^ error: Missing required keyword argument `c`
    requires_mixed 1, 2
    #                  ^ error: Missing required keyword argument `c`
    requires_mixed 1, 2, 3
    #                    ^ error: Too many positional arguments
    #                     ^ error: Missing required keyword argument `c`

    requires_mixed
    #             ^ error: Not enough arguments
    #             ^ error: Missing required keyword argument
    requires_mixed {}
    #             ^ error: Not enough arguments
    #             ^ error: Missing required keyword argument
    requires_mixed()
    #              ^ error: Not enough arguments
    #              ^ error: Missing required keyword argument
    requires_mixed( )
    #               ^ error: Not enough arguments
    #               ^ error: Missing required keyword argument
    requires_mixed() {}
    #              ^ error: Not enough arguments
    #              ^ error: Missing required keyword argument
    requires_mixed(  ) {}
    #                ^ error: Not enough arguments
    #                ^ error: Missing required keyword argument
    requires_mixed(0)
    #               ^ error: Not enough arguments
    #               ^ error: Missing required keyword argument
    requires_mixed(0, )
    #               ^ error: Not enough arguments
    #               ^ error: Missing required keyword argument
    requires_mixed(any:)
    #                  ^ error: Not enough arguments
    #                  ^ error: Missing required keyword argument
    requires_mixed(any:,)
    #                  ^ error: Not enough arguments
    #                  ^ error: Missing required keyword argument
    requires_mixed((0))
    #                ^ error: Not enough arguments
    #                ^ error: Missing required keyword argument
    requires_mixed((0), (1))
    #                     ^ error: Missing required keyword argument
    requires_mixed(0, (1))
    #                   ^ error: Missing required keyword argument
    requires_mixed((0), 1)
    #                    ^ error: Missing required keyword argument
  end

  def requires_multiple_kwarg(arg0, x:, y:)
  end

  def call_requires_multiple_kwarg(x)
    requires_multiple_kwarg
    #                      ^ error: Not enough arguments
    #                      ^ error: Missing 2 required keyword arguments
    requires_multiple_kwarg {}
    #                      ^ error: Not enough arguments
    #                      ^ error: Missing 2 required keyword arguments
    requires_multiple_kwarg()
    #                       ^ error: Not enough arguments
    #                       ^ error: Missing 2 required keyword arguments
    requires_multiple_kwarg( )
    #                        ^ error: Not enough arguments
    #                        ^ error: Missing 2 required keyword arguments
    requires_multiple_kwarg() {}
    #                       ^ error: Not enough arguments
    #                       ^ error: Missing 2 required keyword arguments
    requires_multiple_kwarg(  ) {}
    #                         ^ error: Not enough arguments
    #                         ^ error: Missing 2 required keyword arguments
    requires_multiple_kwarg(0)
    #                        ^ error: Missing 2 required keyword arguments
    requires_multiple_kwarg(0, )
    #                        ^ error: Missing 2 required keyword arguments
    requires_multiple_kwarg(any:)
    #                           ^ error: Missing 2 required keyword arguments
    requires_multiple_kwarg(any:,)
    #                           ^ error: Missing 2 required keyword arguments
    requires_multiple_kwarg((0))
    #                         ^ error: Missing 2 required keyword arguments
    requires_multiple_kwarg((0), (1))
    #                             ^ error: Too many positional
    #                              ^ error: Missing 2 required keyword arguments
    requires_multiple_kwarg(0, (1))
    #                           ^ error: Too many positional
    #                            ^ error: Missing 2 required keyword arguments
    requires_multiple_kwarg((0), 1)
    #                            ^ error: Too many positional
    #                             ^ error: Missing 2 required keyword arguments
  end
end
