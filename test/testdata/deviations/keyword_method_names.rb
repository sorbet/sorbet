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
    alias # parser-error: unexpected token
    # need these here because Sorbet will try to parse 'alias end def' otherwise
    foo bar
end

def method_named_and(x)
  x.and # ok
  x
    .and # ok
  x.
    and # parser-error: unexpected token
end

def method_named_begin(x)
  x.begin # ok
  x
    .begin # ok
  x.
    begin # parser-error: unexpected token

  # due to how we've chosen to deviate, the above begin will look like a normal
  # begin token so we need an extra end token to balance it.
  end
end

def method_named_break(x)
  x.break # ok
  x
    .break # ok
  x.
    break # parser-error: unexpected token
end

def method_named_case(x)
  x.case # ok
  x
    .case # ok
  x.
    case
  # ^^^^ parser-error: unexpected token "case"
  # ^^^^ parser-error: "case" statement must at least have one "when" clause
  # ^^^^ parser-error: Hint: this "case" token might not be properly closed
end

def method_named_class(x)
  x.class # ok
  x
    .class # ok
  x.
    class # parser-error: unexpected token
end

def method_named_defined?(x)
  x.defined? # ok
  x
    .defined? # ok
  x.
    defined? # parser-error: unexpected token
end # parser-error: unexpected token

def method_named_do(x)
  x.do # ok
  x
    .do # ok
  x.
    do # parser-error: unexpected token
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
    elsif # parser-error: unexpected token
end

def method_named_ensure(x)
  x.ensure # ok
  x
    .ensure # ok
  x.
    ensure # parser-error: unexpected token
end

def method_named_false(x)
  x.false # ok
  x
    .false # ok
  x.
    false # parser-error: unexpected token
end

def method_named_for(x)
  x.for # ok
  x
    .for # ok
  x.
    for # parser-error: unexpected token
end

def method_named_if(x)
  x.if # ok
  x
    .if # ok
  x.
    if # parser-error: unexpected token
end

def method_named_in(x)
  x.in # ok
  x
    .in # ok
  x.
    in # parser-error: unexpected token
end

def method_named_module(x)
  x.module # ok
  x
    .module # ok
  x.
    module # parser-error: unexpected token
end

def method_named_next(x)
  x.next # ok
  x
    .next # ok
  x.
    next # parser-error: unexpected token
end

def method_named_nil(x)
  x.nil # ok
  x
    .nil # ok
  x.
    nil # parser-error: unexpected token
end

def method_named_not(x)
  x.not # ok
  x
    .not # ok
  x.
    not # parser-error: unexpected token
end # parser-error: unexpected token

def method_named_or(x)
  x.or # ok
  x
    .or # ok
  x.
    or # parser-error: unexpected token
end

def method_named_redo(x)
  x.redo # ok
  x
    .redo # ok
  x.
    redo
  # ^^^^ parser-error: unexpected token "redo"
  # ^^^^ error: Unsupported node type `Redo`
end

def method_named_rescue(x)
  x.rescue # ok
  x
    .rescue # ok
  x.
    rescue # parser-error: unexpected token
end

def method_named_retry(x)
  x.retry # ok
  x
    .retry # ok
  x.
    retry # parser-error: unexpected token
end

def method_named_return(x)
  x.return # ok
  x
    .return # ok
  x.
    return # parser-error: unexpected token
end

def method_named_self(x)
  x.self # ok
  x
    .self # ok
  x.
    self # parser-error: unexpected token
end

def method_named_super(x)
  x.super # ok
  x
    .super # ok
  x.
    super # parser-error: unexpected token
end

def method_named_then(x)
  x.then # ok
  x
    .then # ok
  x.
    then # parser-error: unexpected token
end

def method_named_true(x)
  x.true # ok
  x
    .true # ok
  x.
    true # parser-error: unexpected token
end

def method_named_unless(x)
  x.unless # ok
  x
    .unless # ok
  x.
    unless # parser-error: unexpected token
end

def method_named_until(x)
  x.until # ok
  x
    .until # ok
  x.
    until # parser-error: unexpected token
end

def method_named_when(x)
  x.when # ok
  x
    .when # ok
  x.
    when # parser-error: unexpected token
end

def method_named_while(x)
  x.while # ok
  x
    .while # ok
  x.
    while # parser-error: unexpected token
end
def method_named_yield(x)
  x.yield # ok
  x
    .yield # ok
  x.
    yield # parser-error: unexpected token
end

# This test has to be last, because otherwise it screws up all the other tests
# (end is special). We can't test `x.def` in this file for similar reasons.
def method_named_end(x)
  x.end # ok
  x
    .end # ok
  x.
    end # parser-error: unexpected token
end # parser-error: unexpected token
