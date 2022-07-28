# typed: true

xs = T::Array[T.proc.void].new
T.reveal_type(xs) # error: `T::Array[T.proc.void]`
xs = T::Array[T.proc.returns(Integer)].new
T.reveal_type(xs) # error: `T::Array[T.proc.returns(Integer)]`
T.reveal_type(xs.fetch(0).call) # error: `Integer`
xs = T::Array[T.proc.params(x: String).returns(Integer)].new
T.reveal_type(xs) # error: `T::Array[T.proc.params(arg0: String).returns(Integer)]`

xs = T::Array[T.proc.params(String).void].new
#                           ^^^^^^ error: Too many positional arguments provided for method `T.proc.params`. Expected: `0`, got: `1`
T.reveal_type(xs) # error: `T::Array[T.proc.void]`

xs = T::Array[T.proc.params(x: String, y:).void].new
#                                      ^^ error: Method `y` does not exist on `T.class_of(<root>)`
T.reveal_type(xs) # error: `T::Array[T.proc.params(arg0: String, arg1: T.untyped).void]`

xs = T::Array[T.proc.params(y: x: String).void].new
#                           ^^ error: unexpected token tLABEL
T.reveal_type(xs) # error: `T::Array[T.proc.params(arg0: T.untyped, arg1: String).void]`

args = T::Array[T.untyped].new
xs = T::Array[T.proc.params(*args).void].new # error: Splats are only supported where the size of the array is known statically
T.reveal_type(xs) # error: `T::Array[T.untyped]`

opts = T::Hash[T.untyped, T.untyped].new
xs = T::Array[T.proc.params(**opts).void].new
T.reveal_type(xs) # error: `T::Array[T.proc.void]`

class A
  extend T::Sig
  # Should be allowed to use bind out of order
  sig {params(blk: T.proc.void.bind(T.self_type)).void}
  def example(&blk)
  end
end

A.new.example do
  T.reveal_type(self) # error: `A`
end
