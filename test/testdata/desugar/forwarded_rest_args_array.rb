# typed: true

def forwarded_rest_args(*)
  xs = [:before, *, :after]
  T.reveal_type(xs) # error: `T::Array[T.untyped]`
end

def start_dotdotdot(*, ...)
  #                 ^ error: ... after rest argument
end
