# typed: true
# enable-suggest-unsafe: true
extend T::Sig

sig {params(c: T::Boolean, x: T.untyped).returns(Integer)}
def multi_line(c, x)
  if c
    x
  else
    "not an integer" # error: Expected `Integer` but found `String("not an integer")` for method result type
  end
end

sig {params(c: T::Boolean, u: T.untyped).returns(String)}
def multi_statement(c, u)
  if c
    puts
    0 # error: Expected `String` but found `Integer(0)` for method result type
  else
    u
  end
end

sig {params(c: T::Boolean, u: T.untyped).returns(String)}
def parenthesized(c, u)
  if c then (puts; 0) else u end # error: Expected `String` but found `Integer(0)` for method result type
end

sig {params(c: T::Boolean, u: T.untyped).returns(String)}
def ternary(c, u)
  c ? 0 : u # error: Expected `String` but found `Integer(0)` for method result type
end

sig {params(c: T::Boolean, u: T.untyped).returns(String)}
def unless_else(c, u)
  unless c then 0 else u end # error: Expected `String` but found `Integer(0)` for method result type
end

sig {params(x: Integer, u: T.untyped).returns(String)}
def elsif_branch(x, u)
  if x == 1
    u
  elsif x == 2
    0 # error: Expected `String` but found `Integer(0)` for method result type
  else
    u
  end
end

sig {params(x: Symbol, u: T.untyped).returns(String)}
def when_branch(x, u)
  case x
  when :a then 0 # error: Expected `String` but found `Integer(0)` for method result type
  else u
  end
end

sig {params(u: T.untyped).returns(String)}
def and_and_truthy(u)
  u.foo? && 0
# ^^^^^^ error: Expected `String` but found `T.nilable(FalseClass)` for method result type
#           ^ error: Expected `String` but found `Integer(0)` for method result type
end

sig {params(u: T.untyped).returns(String)}
def or_or_falsy(u)
  u || 0 # error: Expected `String` but found `Integer(0)` for method result type
end

sig {params(x: T.untyped, y: T.untyped).returns(T::Boolean)}
def and_and_negated(x, y)
  !x && !y # error: Expected `T::Boolean` but found `T.nilable(FalseClass)` for method result type
end

sig {params(x: T.untyped, y: T.untyped).returns(T::Boolean)}
def and_and_comparison(x, y)
  x == 1 && y # error: Expected `T::Boolean` but found `T.nilable(FalseClass)` for method result type
end

sig {params(c: T::Boolean).returns(Integer)}
def modifier_if_both(c)
  "str" if c
# ^^^^^ error: Expected `Integer` but found `String("str")` for method result type
#           ^ error: Expected `Integer` but found `NilClass` for method result type
end

sig {params(x: T.nilable(Integer)).returns(T.noreturn)}
def modifier_if(x)
  raise "boom" if x # error: Expected `T.noreturn` but found `NilClass` for method result type
end

sig {params(x: T.nilable(Integer)).returns(Integer)}
def modifier_unless(x)
  x unless x.nil? # error: Expected `Integer` but found `NilClass` for method result type
end

sig {params(c: T::Boolean, d: T::Boolean, u: T.untyped).returns(String)}
def nested_if(c, d, u)
  if c
    if d then 0 else u end # error: Expected `String` but found `Integer(0)` for method result type
  else
    :sym # error: Expected `String` but found `Symbol(:sym)` for method result type
  end
end

sig {params(t: Symbol, x: T.nilable(String)).returns(String)}
def nested_modifier_if(t, x)
  case t
  when :a
    x if x # error: Expected `String` but found `NilClass` for method result type
  else
    t # error: Expected `String` but found `Symbol` for method result type
  end
end

sig {params(t: Symbol, c: T::Boolean, u: T.untyped).returns(String)}
def nested_in_missing_else(t, c, u)
  case t
  when :a
    if c then 0 else u end # error: Expected `String` but found `Integer(0)` for method result type
  end # error: Expected `String` but found `NilClass` for method result type
end

sig {params(x: T.untyped).returns(String)}
def condition_in_branch(x)
  if x
    0 # error: Expected `String` but found `Integer(0)` for method result type
  else
    x # error: Expected `String` but found `T.nilable(FalseClass)` for method result type
  end
end
