# typed: true
extend T::Sig

sig { params(a: Integer, b: Integer, c: Integer).returns(String) }
def pos_args(a, b, c); a.to_s; end
def req_kwargs(a:, b:, c:); end
def opt_kwargs(a: 1, b: 2, c: 3); end
def all_the_args(a, b: 2, &blk); end

def foo(*)
  res = pos_args(*)
  #     ^^^^^^^^^^^ error: Splats are only supported where the size of the array is known statically
  T.reveal_type(res) # error: `T.untyped`

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

sig { params("**": Integer).void }
def buzz(**); end
buzz(a: 1)
buzz(a: "hello")
#    ^^^^^^^^^^ error: Expected `Integer` but found `String("hello")` for argument `"**":`
