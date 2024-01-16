# typed: false

# We intentionally deviate from Ruby syntax in these cases, namely when a
# method call is broken across two lines between the '.' and the name of the
# method. We tested this on Stripe's codebase and found no call sites where a
# keyword was both (1) being used as a method name and (2) broken across two
# lines like this.
#
# We decided that this was a worthwhile tradeoff, because it makes it far easier
# for the parser to recover gracefully and locally from syntax errors.

def method_named_alias(x)
  x.alias # ok
  x
    .alias # ok
  x.
    alias # error: unexpected token
    # need these here because Sorbet will try to parse 'alias end def' otherwise
    foo bar
end

def method_named_and(x)
  x.and # ok
  x
    .and # ok
  x.
    and # error: unexpected token
end

def method_named_begin(x)
  x.begin # ok
  x
    .begin # ok
  x.
    begin # error: unexpected token

  # due to how we've chosen to deviate, the above begin will look like a normal
  # begin token so we need an extra end token to balance it.
  end
end

def method_named_break(x)
  x.break # ok
  x
    .break # ok
  x.
    break # error: unexpected token
end

def method_named_case(x)
  x.case # ok
  x
    .case # ok
  x.
    case
  # ^^^^ error: unexpected token "case"
  # ^^^^ error: "case" statement must at least have one "when" clause
  # ^^^^ error: Hint: this "case" token might not be properly closed
end

def method_named_class(x)
  x.class # ok
  x
    .class # ok
  x.
    class # error: unexpected token
end

def method_named_defined?(x)
  x.defined? # ok
  x
    .defined? # ok
  x.
    defined? # error: unexpected token
end # error: unexpected token

def method_named_do(x)
  x.do # ok
  x
    .do # ok
  x.
    do # error: unexpected token
end

def method_named_else(x)
  x.else # ok
  x
    .else # ok
end

def method_named_elsif(x)
  x.elsif # ok
  x
    .elsif # ok
  x.
    elsif # error: unexpected token
end

def method_named_ensure(x)
  x.ensure # ok
  x
    .ensure # ok
  x.
    ensure # error: unexpected token
end

def method_named_false(x)
  x.false # ok
  x
    .false # ok
  x.
    false # error: unexpected token
end

def method_named_for(x)
  x.for # ok
  x
    .for # ok
  x.
    for # error: unexpected token
end

def method_named_if(x)
  x.if # ok
  x
    .if # ok
  x.
    if # error: unexpected token
end

def method_named_in(x)
  x.in # ok
  x
    .in # ok
  x.
    in # error: unexpected token
end

def method_named_module(x)
  x.module # ok
  x
    .module # ok
  x.
    module # error: unexpected token
end

def method_named_next(x)
  x.next # ok
  x
    .next # ok
  x.
    next # error: unexpected token
end

def method_named_nil(x)
  x.nil # ok
  x
    .nil # ok
  x.
    nil # error: unexpected token
end

def method_named_not(x)
  x.not # ok
  x
    .not # ok
  x.
    not # error: unexpected token
end # error: unexpected token

def method_named_or(x)
  x.or # ok
  x
    .or # ok
  x.
    or # error: unexpected token
end

def method_named_redo(x)
  x.redo # ok
  x
    .redo # ok
  x.
    redo
  # ^^^^ error: unexpected token "redo"
  # ^^^^ error: Unsupported node type `Redo`
end

def method_named_rescue(x)
  x.rescue # ok
  x
    .rescue # ok
  x.
    rescue # error: unexpected token
end

def method_named_retry(x)
  x.retry # ok
  x
    .retry # ok
  x.
    retry # error: unexpected token
end

def method_named_return(x)
  x.return # ok
  x
    .return # ok
  x.
    return # error: unexpected token
end

def method_named_self(x)
  x.self # ok
  x
    .self # ok
  x.
    self # error: unexpected token
end

def method_named_super(x)
  x.super # ok
  x
    .super # ok
  x.
    super # error: unexpected token
end

def method_named_then(x)
  x.then # ok
  x
    .then # ok
  x.
    then # error: unexpected token
end

def method_named_true(x)
  x.true # ok
  x
    .true # ok
  x.
    true # error: unexpected token
end

def method_named_unless(x)
  x.unless # ok
  x
    .unless # ok
  x.
    unless # error: unexpected token
end

def method_named_until(x)
  x.until # ok
  x
    .until # ok
  x.
    until # error: unexpected token
end

def method_named_when(x)
  x.when # ok
  x
    .when # ok
  x.
    when # error: unexpected token
end

def method_named_while(x)
  x.while # ok
  x
    .while # ok
  x.
    while # error: unexpected token
end
def method_named_yield(x)
  x.yield # ok
  x
    .yield # ok
  x.
    yield # error: unexpected token
end

# This test has to be last, because otherwise it screws up all the other tests
# (end is special). We can't test `x.def` in this file for similar reasons.
def method_named_end(x)
  x.end # ok
  x
    .end # ok
  x.
    end # error: unexpected token
end # error: unexpected token
