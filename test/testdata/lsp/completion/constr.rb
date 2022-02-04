# typed: true

extend T::Sig

xs = T.let([], T::Array[Integer])

# This completion result is a bit non-sensical at the moment (it doesn't makes
# sense to return any results there) but at least it doesn't crash Sorbet like
# it used to. If you made a change that improves this test, feel free to delete
# this comment.

xs.each.map do |x|
  x
end
#  ^ completion: each, ...
