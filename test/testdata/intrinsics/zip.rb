# typed: true

s = [1].zip([2], [3])
T.reveal_type(s) # error: Revealed type: `T::Array[[Integer, T.nilable(Integer), T.nilable(Integer)]]`

xs = T.let([1,2,3], T::Array[Integer])
res = xs.zip(xs) do |zipped|
  T.reveal_type(zipped) # error: Revealed type: `[Integer, T.nilable(Integer)] (2-tuple)`
end
T.reveal_type(res) # error: Revealed type: `NilClass`

res = xs.zip(xs, xs) do |zipped|
  T.reveal_type(zipped) # error: Revealed type: `[Integer, T.nilable(Integer), T.nilable(Integer)] (3-tuple)`
end

ys = T.let([""], T::Array[String])
zs = T.let([false, true], T::Array[T::Boolean])
res = xs.zip(ys, zs, xs) do |zipped|
  T.reveal_type(zipped) # error: Revealed type: `[Integer, T.nilable(String), T.nilable(T::Boolean), T.nilable(Integer)] (4-tuple)`
end

