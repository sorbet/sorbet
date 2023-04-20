# typed: true

def pos_args(a, b, c); end
def req_kwargs(a:, b:, c:); end
def opt_kwargs(a: 1, b: 2, c: 3); end
def all_the_args(a, b: 2, &blk); end

def foo(*)
  pos_args(*)
# ^^^^^^^^^^^ error: Splats are only supported where the size of the array is known statically

  T.unsafe(self).pos_args(*)
  T.unsafe(self).pos_args(1, *)
  T.unsafe(self).pos_args(1, 2, *)
end

def bar(**)
  req_kwargs(**)
  req_kwargs(a: 1, **)
#            ^^^^^^^^ error: Cannot call `Object#req_kwargs` with a `Hash` keyword splat because the method has required keyword parameters
  req_kwargs(a: 1, b: 2, **)
#            ^^^^^^^^^^^^^^ error: Cannot call `Object#req_kwargs` with a `Hash` keyword splat because the method has required keyword parameters

  opt_kwargs(**)
  opt_kwargs(a: 1, **)
  opt_kwargs(a: 1, b: 2, **)
end

def baz(*, **, &)
  all_the_args(*, **, &)
# ^^^^^^^^^^^^^^^^^^^^^^ error: Splats are only supported where the size of the array is known statically
  T.unsafe(self).all_the_args(*, **, &)
  all_the_args(1, **, &)
  all_the_args(1, b: 3, &)
end
