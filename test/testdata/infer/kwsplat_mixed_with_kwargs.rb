# typed: strict
extend T::Sig

sig {params(x: Integer, y: String).void}
def takes_one_int_one_str(x: 0, y: ''); end

sig {params(x: Integer, y: Integer).void}
def takes_two_ints(x: 0, y: 0); end

sig {params(x: Integer, y: Integer).void}
def takes_required_int(x:, y: 0); end

sig {params(x: Integer, rest: Integer).void}
def takes_kwsplat_param(x: 0, **rest); end

sig do
  params(
    int_opts: T::Hash[Symbol, Integer],
    str_opts: T::Hash[Symbol, String],
    untyped_opts: T::Hash[Symbol, T.untyped],
  )
  .void
end
def example(int_opts, str_opts, untyped_opts)
  # A keyword arg written at the call site is known precisely, so it's checked against its
  # parameter even though a splat is passed along with it.
  takes_two_ints(x: 'not an int', **int_opts)
  #                 ^^^^^^^^^^^^ error: Expected `Integer` but found `String("not an int")` for argument `x`
  takes_two_ints(x: 0, **int_opts)

  # This holds even when nothing can be learned from the splat itself.
  takes_two_ints(x: 'not an int', **untyped_opts)
  #                 ^^^^^^^^^^^^ error: Expected `Integer` but found `String("not an int")` for argument `x`
  takes_two_ints(x: 0, **untyped_opts)

  # The splat trails the explicit keys, so it can overwrite them. Its values still have to fit
  # every keyword parameter, including the ones that were passed explicitly.
  takes_one_int_one_str(y: 'ok', **str_opts)
  #                                ^^^^^^^^ error: Expected `Integer` for keyword parameter `x` but found `String` from keyword splat
  takes_two_ints(x: 0, **str_opts)
  #                      ^^^^^^^^ error: Expected `Integer` for keyword parameter `x` but found `String` from keyword splat

  # `x:` is supplied explicitly, so it no longer counts as being left up to the splat.
  takes_required_int(x: 0, **int_opts)
  takes_required_int(x: 'not an int', **int_opts)
  #                     ^^^^^^^^^^^^ error: Expected `Integer` but found `String("not an int")` for argument `x`
  takes_required_int(**int_opts)
  #                  ^^^^^^^^^^ error: Cannot call `Object#takes_required_int` with a `Hash` keyword splat because the method has required keyword parameters

  # Keys the splat supplies that aren't named parameters land on `**rest`.
  takes_kwsplat_param(x: 0, **int_opts)
  takes_kwsplat_param(x: 'not an int', **int_opts)
  #                      ^^^^^^^^^^^^ error: Expected `Integer` but found `String("not an int")` for argument `x`

  # A splat that isn't in trailing position, and a call with more than one splat, keep being
  # merged into a single hash rather than checked one key at a time.
  takes_two_ints(**int_opts, x: 'not an int')
  takes_two_ints(x: 'not an int', **int_opts, **untyped_opts)
end
