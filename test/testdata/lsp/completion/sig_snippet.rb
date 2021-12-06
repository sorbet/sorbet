# typed: true
extend T::Sig

sig {params(f: T.proc.params(y: T.untyped).returns(T.untyped)).void}
def takes_untyped_proc(f)
end

sig {params(f: String).void}
def takes_string(f)
end

sig {params(f: T::Array[T.untyped]).void}
def takes_array_untyped(f)
end

sig {params(x: {y: String}).void}
def takes_shape(x)
end

sig # error: no block
#  ^ apply-completion: [A] item: 0
def passes_untyped_proc(x, f)
  takes_untyped_proc(x)
  T.unsafe(nil)
end

sig # error: no block
#  ^ apply-completion: [B] item: 0
def passes_string(x)
  takes_string(x)
  T.unsafe(nil)
end

sig # error: no block
#  ^ apply-completion: [C] item: 0
def passes_array_untyped(xs)
  takes_array_untyped(xs)
  T.unsafe(nil)
end

sig # error: no block
#  ^ apply-completion: [D] item: 0
def passes_shape(x)
  takes_shape(x)
  T.unsafe(nil)
end
