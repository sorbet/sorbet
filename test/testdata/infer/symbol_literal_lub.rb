# typed: true

any_symbol = T.let(:abc, Symbol)

T.reveal_type(
  if T.unsafe(nil)
    :abc
  else
    :def
  end
) # error: Revealed type: `T.any(Symbol(:abc), Symbol(:def))`

T.reveal_type(
  if T.unsafe(nil)
    :abc
  else
    any_symbol
  end
) # error: Revealed type: `Symbol`

T.reveal_type(
  if T.unsafe(nil)
    any_symbol
  else
    :abc
  end
) # error: Revealed type: `Symbol`

T.reveal_type(
  if T.unsafe(nil)
    :abc
  else
    "def"
  end
) # error: Revealed type: `T.any(Symbol(:abc), String("def"))`

T.reveal_type(
  if T.unsafe(nil)
    "abc"
  else
    "def"
  end
) # error: Revealed type: `T.any(String("abc"), String("def"))`

T.reveal_type(
  if T.unsafe(nil)
    1
  else
    2
  end
) # error: Revealed type: `T.any(Integer(1), Integer(2))`

T.reveal_type(
  if T.unsafe(nil)
    1.0
  else
    2.0
  end
) # error: Revealed type: `T.any(Float(1.000000), Float(2.000000))`

any_string = T.let("", String)

T.reveal_type(
  if T.unsafe(nil)
    :abc
  else
    any_string
  end
) # error: Revealed type: `T.any(String, Symbol(:abc))`

T.reveal_type(
  if T.unsafe(nil)
    any_string
  else
    :abc
  end
) # error: Revealed type: `T.any(String, Symbol(:abc))`

any_object = T.let(Object.new, Object)

T.reveal_type(
  if T.unsafe(nil)
    :abc
  else
    any_object
  end
) # error: Revealed type: `Object`

T.reveal_type(
  if T.unsafe(nil)
    any_object
  else
    :abc
  end
) # error: Revealed type: `Object`

class Unrelated; end
unrelated = T.let(Unrelated.new, Unrelated)

T.reveal_type(
  if T.unsafe(nil)
    :abc
  else
    unrelated
  end
) # error: Revealed type: `T.any(Unrelated, Symbol(:abc))`

T.reveal_type(
  if T.unsafe(nil)
    unrelated
  else
    :abc
  end
) # error: Revealed type: `T.any(Unrelated, Symbol(:abc))`

tuple = [nil]

T.reveal_type(
  if T.unsafe(nil)
    :abc
  else
    tuple
  end
) # error: Revealed type: `T.any(Symbol(:abc), [NilClass])`

T.reveal_type(
  if T.unsafe(nil)
    tuple
  else
    :abc
  end
) # error: Revealed type: `T.any(Symbol(:abc), [NilClass])`

T.reveal_type(
  if T.unsafe(nil)
    if T.unsafe(nil)
      :abc
    else
      :def
    end
  else
    :ghi
  end
) # error: Revealed type: `T.any(Symbol(:ghi), Symbol(:abc), Symbol(:def))`

T.reveal_type(
  if T.unsafe(nil)
    if T.unsafe(nil)
      :abc
    else
      :def
    end
  else
    any_symbol
  end
) # error: Revealed type: `Symbol`

T.reveal_type(
  if T.unsafe(nil)
    if T.unsafe(nil)
      :abc
    else
      :def
    end
  else
    1
  end
) # error: Revealed type: `T.any(Integer(1), Symbol(:abc), Symbol(:def))`

T.reveal_type([:abc, :def].sample) # error: Revealed type: `Symbol`
T.reveal_type(["abc", "def"].sample) # error: Revealed type: `String`
T.reveal_type([1, 2].sample) # error: Revealed type: `Integer`
T.reveal_type([1.0, 2.0].sample) # error: Revealed type: `Float`
T.reveal_type([[:abc, 1], [:def, 2]].sample) # error: Revealed type: `[Symbol, Integer] (2-tuple)`

strings = [T.unsafe(nil) ? "abc" : "def"]
strings << "ghi"
T.reveal_type(strings.first) # error: Revealed type: `T.any(String("abc"), String("def"))`

T.reveal_type(
  if T.unsafe(nil)
    [:abc, 1]
  else
    [:def, 2]
  end
) # error: Revealed type: `[Symbol, Integer] (2-tuple)`

T.reveal_type(
  if T.unsafe(nil)
    {key: "abc"}
  else
    {key: "def"}
  end
) # error: Revealed type: `{key: String} (shape of T::Hash[T.untyped, T.untyped])`

T.reveal_type(
  if T.unsafe(nil)
    [1]
  else
    [2]
  end
) # error: Revealed type: `[Integer] (1-tuple)`

T.reveal_type(
  if T.unsafe(nil)
    {key: 1}
  else
    {key: 2}
  end
) # error: Revealed type: `{key: Integer} (shape of T::Hash[T.untyped, T.untyped])`

local_array = [1, 2]
T.reveal_type(local_array) # error: Revealed type: `[Integer(1), Integer(2)] (2-tuple)`

LITERAL_ARRAY = [1, 2]
T.reveal_type(LITERAL_ARRAY) # error: Revealed type: `T::Array[Integer]`

# TODO: Explicit annotations should not broaden literal types when assigning to a local.
# x = T.let(:heads, T.any(:heads, :tails))
# T.reveal_type(x) # expected type: `T.any(Symbol(:heads), Symbol(:tails))`

# TODO: add this case back in and fix it later
# extend T::Sig
# sig do
#   type_parameters(:U)
#     .params(left: T.type_parameter(:U), right: T.type_parameter(:U))
#     .returns(T.type_parameter(:U))
# end
# def pick(left, right)
#   left
# end
#
# T.reveal_type(pick(:abc, :def)) # expected type: `T.any(Symbol(:abc), Symbol(:def))`
