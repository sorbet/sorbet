# typed: true
xs = T.let([0], T::Array[Integer])
nil_xs = T.let(xs, T::Array[T.nilable(Integer)])
nil_xs[0] = nil

T.reveal_type(xs.fetch(0)) # type: Integer (!)
puts xs.fetch(0)           # => nil        (!)

