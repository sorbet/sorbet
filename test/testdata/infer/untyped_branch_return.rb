# typed: true
extend T::Sig

# When a method ends with a conditional, the types of its branches are joined
# before being checked against the method's return type. An untyped branch makes
# the joined type untyped, which hides errors in the other branches.

sig {params(u: T.untyped, s: String).returns(Integer)}
def if_else(u, s)
  if Random.rand(2).even?
    s
  else
    u
  end
end

sig {params(u: T.untyped).returns(Integer)}
def if_else_literal(u)
  if Random.rand(2).even?
    'not an int'
  else
    T.unsafe(nil)
  end
end

sig {params(c: T::Boolean, u: T.untyped).returns(Integer)}
def elsif_chain(c, u)
  if c
    u
  elsif u
    :sym
  else
    2.0
  end
end

sig {params(x: T.any(Integer, String, Symbol), u: T.untyped).returns(Integer)}
def case_when(x, u)
  case x
  when Integer then x
  when String then u
  else x
  end
end

sig {params(x: T.nilable(String)).returns(Integer)}
def or_untyped(x)
  x || T.unsafe(nil)
end

sig {params(x: T.nilable(Integer)).returns(Integer)}
def or_untyped_ok(x)
  x || T.unsafe(nil)
end

sig {params(c: T::Boolean, u: T.untyped).returns(Integer)}
def nested_in_begin(c, u)
  puts
  begin
    puts
    if c
      if u
        u
      else
        'nested'
      end
    else
      u
    end
  end
end

sig {params(c: T::Boolean, u: T.untyped).returns(Integer)}
def explicit_returns(c, u)
  if c
    return 'explicit' # error: Expected `Integer` but found `String("explicit")` for method result type
  else
    return u
  end
end

sig {params(c: T::Boolean, u: T.untyped).void}
def void_method(c, u)
  if c
    'ignored'
  else
    u
  end
end

# The same is true when the conditional's value is assigned to a variable, or
# when it's inside a `begin`/`ensure`, `rescue`, or block.

sig {params(c: T::Boolean, u: T.untyped).returns(Integer)}
def assigned_to_variable(c, u)
  y = if c
    'not checked'
  else
    u
  end
  y
end

sig {params(c: T::Boolean, u: T.untyped).returns(Integer)}
def inside_ensure(c, u)
  begin
    if c
      'not checked'
    else
      u
    end
  ensure
    puts
  end
end

sig {params(c: T::Boolean, u: T.untyped).returns(Integer)}
def inside_rescue(c, u)
  if c
    'not checked'
  else
    u
  end
rescue
  0
end

sig {params(c: T::Boolean, u: T.untyped).returns(T::Array[Integer])}
def inside_block(c, u)
  [1].map do
    if c
      'not checked'
    else
      u
    end
  end
end

# `a && b` is a conditional too: `if a then b else a end`.

sig {params(u: T.untyped).returns(T::Boolean)}
def and_and_untyped(u)
  u.foo? && u.bar?
end

sig {params(x: T.nilable(String), u: T.untyped).returns(String)}
def and_and_typed_left(x, u)
  x && u
end
