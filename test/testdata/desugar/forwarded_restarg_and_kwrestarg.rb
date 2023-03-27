# typed: true

def bar(a, b, c); end

def foo(*)
  bar(*)
# ^^^^^^ error: Splats are only supported where the size of the array is known statically

  T.unsafe(self).bar(*)
  T.unsafe(self).bar(1, *)
  T.unsafe(self).bar(1, 2, *)
end
