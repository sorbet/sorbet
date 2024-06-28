# typed: true
extend T::Sig

sig { params(blk: T.proc.void).returns(Integer) }
sig { params(blk: T.proc.params(x: String).void).returns(String) }
def block_arity_overload(&blk) # error: Refusing to typecheck
end

res = block_arity_overload {}
T.reveal_type(res) # error: `Integer`
res = block_arity_overload do |y|
  T.reveal_type(y) # error: `String`
end
T.reveal_type(res) # error: `String`

sig { params(blk: T.proc.void).returns(Integer) }
sig { params(blk: T.proc.params(x: [String, Integer]).void).returns(String) }
sig { params(blk: T.proc.params(x: [Symbol, Float, T::Boolean]).void).returns(Object) }
def block_arity_overload_tuple(&blk) # error: Refusing to typecheck
end

res = block_arity_overload_tuple do
end
T.reveal_type(res) # error: `Integer`
res = block_arity_overload_tuple do |pair|
  T.reveal_type(pair) # error: `[String, Integer] (2-tuple)`
end
T.reveal_type(res) # error: `String`
res = block_arity_overload_tuple do |x, y|
  T.reveal_type(x) # error: `String`
  T.reveal_type(y) # error: `Integer`
end
T.reveal_type(res) # error: `String`
res = block_arity_overload_tuple do |x, y, z|
  T.reveal_type(x) # error: `Symbol`
  T.reveal_type(y) # error: `Float`
  T.reveal_type(z) # error: `T::Boolean`
end
T.reveal_type(res) # error: `Object`

sig { params(blk: T.proc.void).returns(Integer) }
sig { params(blk: T.proc.params(x: T::Array[Integer]).void).returns(String) }
def block_arity_overload_array(&blk) # error: Refusing to typecheck
end

res = block_arity_overload_array do
end
T.reveal_type(res) # error: `Integer`
res = block_arity_overload_array do |pair|
  T.reveal_type(pair) # error: `T::Array[Integer]`
end
T.reveal_type(res) # error: `String`
res = block_arity_overload_array do |x, y|
  T.reveal_type(x) # error: `T.nilable(Integer)`
  T.reveal_type(y) # error: `T.nilable(Integer)`
end
T.reveal_type(res) # error: `String`

sig { params(blk: T.proc.params(x: String, y: Integer).void).returns(String) }
sig { params(blk: T.proc.params(x: Float).void).returns(Integer) }
def block_arity_overload_bad_order_choice(&blk) # error: Refusing to typecheck
end

res = block_arity_overload_bad_order_choice do
end
T.reveal_type(res) # error: `String`
res = block_arity_overload_bad_order_choice do |x|
  T.reveal_type(x) # error: `String`
end
T.reveal_type(res) # error: `String`
res = block_arity_overload_bad_order_choice do |x, y|
  T.reveal_type(x) # error: `String`
  T.reveal_type(y) # error: `Integer`
end
T.reveal_type(res) # error: `String`

sig { params(blk: T.proc.params(x: Float).void).returns(Integer) }
sig { params(blk: T.proc.params(x: String, y: Integer).void).returns(String) }
def block_arity_overload_good_order_choice(&blk) # error: Refusing to typecheck
end

res = block_arity_overload_good_order_choice do
end
T.reveal_type(res) # error: `Integer`
res = block_arity_overload_good_order_choice do |x|
  T.reveal_type(x) # error: `Float`
end
T.reveal_type(res) # error: `Integer`
res = block_arity_overload_good_order_choice do |x, y|
  T.reveal_type(x) # error: `String`
  T.reveal_type(y) # error: `Integer`
end
T.reveal_type(res) # error: `String`
