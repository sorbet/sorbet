# typed: true
extend T::Sig

sig do
  type_parameters(:U)
    .params(
      x: T.type_parameter(:U),
      blk: T.proc.returns(T.type_parameter(:U))
    )
      .returns(T.type_parameter(:U))
end
def example(x, &blk)
  if [true, false].sample
    return x
  else
    return blk.call
  end
end

f = T.let(->() { 0 }, T.proc.returns(Integer))
res = example('', &f) # error: Expected `T.proc.returns(String)` but found `T.proc.returns(Integer)` for block argument
T.reveal_type(res) # error: `T.any(String, Integer)`


# --- more complicated example ---

sig do
  type_parameters(:U)
    .params(
      exn_class: T.class_of(StandardError)[T.all(StandardError, T.type_parameter(:U))],
      blk: T.proc.params(
        raiser: T.proc.params(exn: T.all(StandardError, T.type_parameter(:U))).void
      ).void
    )
      .void
end
def complicated(exn_class, &blk)
  raiser = T.let(
    -> (exn) do
      T.reveal_type(exn) # error: `T.all(StandardError, T.type_parameter(:U) (of Object#complicated))`
      raise exn
    end,
    T.proc.params(exn: T.all(StandardError, T.type_parameter(:U))).void
  )

  bad_raiser = T.let(
    -> (x) do
      raise x
      #     ^ error: Expected `T.any(T::Class[T.anything], Exception, String)` but found `T.type_parameter(:U) (of Object#complicated)` for argument `arg0`
    end,
    T.proc.params(x: T.type_parameter(:U)).void
  )

  begin
    yield raiser
  rescue exn_class => exn
    T.reveal_type(exn_class) # error: `T.class_of(StandardError)[T.all(StandardError, T.type_parameter(:U) (of Object#complicated))]`
  end

  # This is fine by variance rules. The user will be limited to passing in
  # values for `x` that are instances of `exn_class`, but that's an artificial
  # limitation--this function accepts more values than required (because
  # `T.type_parameter(:U) <: T.anything`).
  technically_good_raiser = T.let(
    -> (x) { raise },
    T.proc.params(x: T.anything).void
  )
  yield technically_good_raiser
end

complicated(TypeError) do |raiser|
  T.reveal_type(raiser) # error: `T.proc.params(arg0: TypeError).void`
  raiser.call(TypeError.new)
  raiser.call(ArgumentError.new) # error: Expected `TypeError` but found `ArgumentError` for argument `arg0`
end
