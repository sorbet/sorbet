# typed: true
extend T::Sig

# When a method ends with a conditional, each branch is checked against the
# method's return type separately, so that an untyped branch does not hide errors
# in the other branches.

sig {params(u: T.untyped, s: String).returns(Integer)}
def if_else(u, s)
  if Random.rand(2).even?
    s # error: Expected `Integer` but found `String` for method result type
  else
    u
  end
end

sig {params(u: T.untyped).returns(Integer)}
def if_else_literal(u)
  if Random.rand(2).even?
    'not an int' # error: Expected `Integer` but found `String("not an int")` for method result type
  else
    T.unsafe(nil)
  end
end

sig {params(c: T::Boolean, u: T.untyped).returns(Integer)}
def elsif_chain(c, u)
  if c
    u
  elsif u
    :sym # error: Expected `Integer` but found `Symbol(:sym)` for method result type
  else
    2.0 # error: Expected `Integer` but found `Float(2.000000)` for method result type
  end
end

sig {params(x: T.any(Integer, String, Symbol), u: T.untyped).returns(Integer)}
def case_when(x, u)
  case x
  when Integer then x
  when String then u
  else x # error: Expected `Integer` but found `Symbol` for method result type
  end
end

sig {params(x: T.nilable(String)).returns(Integer)}
def or_untyped(x)
  x || T.unsafe(nil) # error: Expected `Integer` but found `String` for method result type
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
        'nested' # error: Expected `Integer` but found `String("nested")` for method result type
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

# Only tail position is affected. A conditional whose value is assigned to a
# variable, or which is inside a `begin`/`ensure`, `rescue`, or block, still
# joins its branches before the result is checked.

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

# `a && b` is `if a then b else a end`, and its falsy branch returns `a`. When
# `a` is untyped, that's `nil` or `false`.

sig {params(u: T.untyped).returns(T::Boolean)}
def and_and_untyped(u)
  u.foo? && u.bar? # error: Expected `T::Boolean` but found `T.nilable(FalseClass)` for method result type
end

sig {params(x: T.nilable(String), u: T.untyped).returns(String)}
def and_and_typed_left(x, u)
  x && u # error: Expected `String` but found `NilClass` for method result type
end
