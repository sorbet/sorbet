# typed: true

extend T::Sig

xs = T.let([], T::Array[Integer])

xs.each.map do |x|
  #           ^ completion: (nothing)
  x
  ##
  # ^ completion: (nothing)
end
#  ^ completion: (nothing)


1.times do |x|
  # x.
  #   ^ completion: (nothing)
end
