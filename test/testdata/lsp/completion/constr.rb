# typed: true

extend T::Sig

xs = T.let([], T::Array[Integer])

xs.each.map do |x|
  x
end
#  ^ completion: (nothing)
