# typed: true
extend T::Sig

sig {void}
def assign_before_let
  b = :sym
  b = T.let("str", String)
  b = :sym # error: Incompatible assignment to variable declared via `let`: `Symbol(:sym)` is not a subtype of `String`
end

sig {params(c: T::Boolean).void}
def let_in_then_branch(c)
  if c
    s = T.let("", String)
  else
    s = ""
  end
  T.reveal_type(s) # error: Revealed type: `String`
  s = 1 # error: Incompatible assignment to variable declared via `let`: `Integer(1)` is not a subtype of `String`
end

sig {params(c: T::Boolean).void}
def let_in_else_branch(c)
  if c
    s = ""
  else
    s = T.let("", String)
  end
  T.reveal_type(s) # error: Revealed type: `String`
  s = 1 # error: Incompatible assignment to variable declared via `let`: `Integer(1)` is not a subtype of `String`
end

sig {params(xs: T::Array[String]).void}
def let_in_block(xs)
  foo = "memory"
  xs.each {|s| foo = T.let(s, String)}
end

sig {params(x: T.nilable(Integer)).void}
def let_after_or(x)
  y = x || T.let(0, Integer)
  T.reveal_type(y) # error: Revealed type: `Integer`
end

sig {params(c: T::Boolean).void}
def let_in_assigned_if(c)
  z = if c
    0
  else
    T.let(0, Integer)
  end
  T.reveal_type(z) # error: Revealed type: `Integer`
end

# A write before the `T.let` in a loop body isn't checked against the
# `T.let`'s type.
sig {params(c: T::Boolean).void}
def write_before_let_in_loop(c)
  while c
    x = 1
    x = T.let("s", String)
  end
end

sig {params(c: T::Boolean, d: T::Boolean).void}
def let_in_one_branch_in_loop(c, d)
  x = 0
  while c
    if d
      x = T.let(1, Integer)
    else
      x = "s" # error: Changing the type of a variable is not permitted in loops and blocks
    end
  end
  T.reveal_type(x) # error: Revealed type: `Integer(0)`
end

sig {params(xs: T::Array[Integer]).void}
def type_change_in_block_before_let(xs)
  x = 1
  xs.each { x = "str" } # error: Changing the type of a variable is not permitted in loops and blocks
  T.reveal_type(x) # error: Revealed type: `Integer(1)`
  x = T.let(2, Integer)
end

# A `T.let` in a block doesn't pin the variable after the block.
sig {params(xs: T::Array[Integer]).void}
def write_after_let_in_block(xs)
  x = 0
  xs.each { x = T.let(1, Integer) }
  x = "s"
  T.reveal_type(x) # error: Revealed type: `String("s")`
end

sig {params(c: T::Boolean).void}
def let_in_then_branch_other_type(c)
  if c
    x = T.let(1, Integer)
  else
    x = "s"
  end
  T.reveal_type(x) # error: Revealed type: `T.any(Integer, String)`
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
