# typed: true
extend T::Sig

sig {void}
def assign_before_let
  b = :sym # error: Incompatible assignment to variable declared via `let`: `Symbol(:sym)` is not a subtype of `NilClass`
  b = T.let("str", String)
  b = :sym # error: Incompatible assignment to variable declared via `let`: `Symbol(:sym)` is not a subtype of `String`
end

sig {params(c: T::Boolean).void}
def let_in_then_branch(c)
  if c
    s = T.let("", String)
  else
    s = "" # error: Incompatible assignment to variable declared via `let`: `String("")` is not a subtype of `NilClass`
  end
  T.reveal_type(s) # error: Revealed type: `T.nilable(String)`
  s = 1 # error: Incompatible assignment to variable declared via `let`: `Integer(1)` is not a subtype of `String`
end

sig {params(c: T::Boolean).void}
def let_in_else_branch(c)
  if c
    s = "" # error: Incompatible assignment to variable declared via `let`: `String("")` is not a subtype of `NilClass`
  else
    s = T.let("", String)
  end
  T.reveal_type(s) # error: Revealed type: `T.nilable(String)`
  s = 1 # error: Incompatible assignment to variable declared via `let`: `Integer(1)` is not a subtype of `String`
end

sig {params(xs: T::Array[String]).void}
def let_in_block(xs)
  foo = "memory" # error: Incompatible assignment to variable declared via `let`: `String("memory")` is not a subtype of `NilClass`
  xs.each {|s| foo = T.let(s, String)}
end

sig {params(x: T.nilable(Integer)).void}
def let_after_or(x)
  y = x || T.let(0, Integer) # error: Incompatible assignment to variable declared via `let`: `Integer` is not a subtype of `NilClass`
  T.reveal_type(y) # error: Revealed type: `T.nilable(Integer)`
end

sig {params(c: T::Boolean).void}
def let_in_assigned_if(c)
  z = if c
    0 # error: Incompatible assignment to variable declared via `let`: `Integer(0)` is not a subtype of `NilClass`
  else
    T.let(0, Integer)
  end
  T.reveal_type(z) # error: Revealed type: `T.nilable(Integer)`
end

sig {params(c: T::Boolean).void}
def write_before_let_in_loop(c)
  while c
    x = 1 # error: Incompatible assignment to variable declared via `let`: `Integer(1)` is not a subtype of `NilClass`
    x = T.let("s", String)
  end
end

sig {params(c: T::Boolean, d: T::Boolean).void}
def let_in_one_branch_in_loop(c, d)
  x = 0 # error: Incompatible assignment to variable declared via `let`: `Integer(0)` is not a subtype of `NilClass`
  while c
    if d
      x = T.let(1, Integer)
    else
      x = "s" # error: Incompatible assignment to variable declared via `let`: `String("s")` is not a subtype of `NilClass`
    end
  end
  T.reveal_type(x) # error: Revealed type: `NilClass`
end

sig {params(xs: T::Array[Integer]).void}
def type_change_in_block_before_let(xs)
  x = 1 # error: Incompatible assignment to variable declared via `let`: `Integer(1)` is not a subtype of `NilClass`
  xs.each { x = "str" } # error: Incompatible assignment to variable declared via `let`: `String("str")` is not a subtype of `NilClass`
  T.reveal_type(x) # error: Revealed type: `NilClass`
  x = T.let(2, Integer)
end

sig {params(xs: T::Array[Integer]).void}
def write_after_let_in_block(xs)
  x = 0 # error: Incompatible assignment to variable declared via `let`: `Integer(0)` is not a subtype of `NilClass`
  xs.each { x = T.let(1, Integer) }
  x = "s" # error: Incompatible assignment to variable declared via `let`: `String("s")` is not a subtype of `NilClass`
  T.reveal_type(x) # error: Revealed type: `NilClass`
end

sig {params(c: T::Boolean).void}
def let_in_then_branch_other_type(c)
  if c
    x = T.let(1, Integer)
  else
    x = "s" # error: Incompatible assignment to variable declared via `let`: `String("s")` is not a subtype of `NilClass`
  end
  T.reveal_type(x) # error: Revealed type: `T.nilable(Integer)`
end

# Writing an untyped value in a block is allowed, as it is for a write that a
# `T.let` has pinned.
sig {params(c: T::Boolean, r: T.untyped, xs: T::Array[Integer]).void}
def untyped_write_in_block(c, r, xs)
  if c
    auth = T.let(r.auth, T.untyped)
  else
    xs.each { auth = r.auth }
  end
end

# The write in the block changes `auth`'s type: a read before it is `NilClass`
# on the first iteration and untyped on the next.
sig {params(c: T::Boolean, r: T.untyped, xs: T::Array[Integer]).void}
def read_before_untyped_write_in_block(c, r, xs)
  if c
    auth = T.let(r.auth, T.untyped)
  else
    xs.each do
      T.reveal_type(auth) # error: Revealed type: `NilClass`
      auth = r.auth
    end
  end
end
