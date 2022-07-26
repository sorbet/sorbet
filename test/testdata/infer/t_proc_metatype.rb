# typed: true

# TODO(jez) Test various builder order combinations
# TODO(jez) Test all kinds of pos/kwarg/splat arg combinations
# TODO(jez) This should make typed procs easy to support? Make a ticket:
#
#   T.proc.params(x: Integer).void do |x|
#     T.reveal_type(x)
#   end

xs = T::Array[T.proc.void].new
T.reveal_type(xs)
xs = T::Array[T.proc.returns(Integer)].new
T.reveal_type(xs)
T.reveal_type(xs.fetch(0).call)
xs = T::Array[T.proc.params(x: String).returns(Integer)].new
T.reveal_type(xs)
