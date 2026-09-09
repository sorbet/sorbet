# typed: true

class A
  x = private :foo
  T.reveal_type(x) # error: `Symbol`
  x = private 'foo'
  T.reveal_type(x) # error: `String`
  x = private :foo, :bar
  T.reveal_type(x) # error: `T::Array[Symbol]`
  x = private :foo, 'bar'
  T.reveal_type(x) # error: `T::Array[T.any(Symbol, String)]`
  x = private [:foo, :bar]
  T.reveal_type(x) # error: `T::Array[Symbol]`
  x = private [:foo, 'bar']
  T.reveal_type(x) # error: `T::Array[T.any(Symbol, String)]`

  x = private [:foo], :bar
  #           ^^^^^^ error: Expected `T.all(T.type_parameter(:U), T.any(Symbol, String))` but found `[Symbol(:foo)]` for argument `rest`
  x = private [:foo], [:bar]
  #           ^^^^^^ error: Expected `T.all(T.type_parameter(:U), T.any(Symbol, String))` but found `[Symbol(:foo)]` for argument `rest`
  #                   ^^^^^^ error: Expected `T.all(T.type_parameter(:U), T.any(Symbol, String))` but found `[Symbol(:bar)]` for argument `rest`
end
