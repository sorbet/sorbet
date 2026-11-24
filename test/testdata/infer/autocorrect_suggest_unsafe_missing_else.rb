# typed: true
# enable-suggest-unsafe: true
extend T::Sig

sig {params(c: T::Boolean, u: T.untyped).returns(String)}
def if_multi_line(c, u)
  if c
    u
  end # error: Expected `String` but found `NilClass` for method result type
end

sig {params(c: T::Boolean, u: T.untyped).returns(String)}
def if_then(c, u)
  if c then
    u
  end # error: Expected `String` but found `NilClass` for method result type
end

sig {params(c: T::Boolean, u: T.untyped).returns(String)}
def if_single_line(c, u)
  if c then u end # error: Expected `String` but found `NilClass` for method result type
end

sig {params(c: T::Boolean, u: T.untyped).returns(String)}
def if_semicolons(c, u)
  if c; u; end # error: Expected `String` but found `NilClass` for method result type
end

sig {params(c: T::Boolean, u: T.untyped).returns(String)}
def if_parenthesized_condition(c, u)
  if (c)
    u
  end # error: Expected `String` but found `NilClass` for method result type
end

sig {params(c: T::Boolean, u: T.untyped).returns(String)}
def if_multi_statement(c, u)
  if c
    puts
    u
  end # error: Expected `String` but found `NilClass` for method result type
end

sig {params(c: T::Boolean, u: T.untyped).returns(String)}
def if_comments(c, u)
  if c # comment after the condition
    u
    # comment before the end
  end # error: Expected `String` but found `NilClass` for method result type
end

sig {params(c: T::Boolean, u: T.untyped).returns(String)}
def if_end_after_value(c, u)
  if c
    u end # error: Expected `String` but found `NilClass` for method result type
end

sig {params(x: Integer, u: T.untyped).returns(String)}
def elsif_chain(x, u)
  if x == 1
    u
  elsif x == 2
    u
  end # error: Expected `String` but found `NilClass` for method result type
end

sig {params(c: T::Boolean, u: T.untyped).returns(String)}
def unless_multi_line(c, u)
  unless c
    u
  end # error: Expected `String` but found `NilClass` for method result type
end

sig {params(c: T::Boolean, u: T.untyped).returns(String)}
def unless_single_line(c, u)
  unless c then u end # error: Expected `String` but found `NilClass` for method result type
end

sig {params(x: Integer, u: T.untyped).returns(String)}
def case_when(x, u)
  case x
  when 1 then u
  when 2
    u
  end # error: Expected `String` but found `NilClass` for method result type
end

sig {params(x: Integer, u: T.untyped).returns(String)}
def case_when_multiple_patterns(x, u)
  case x
  when 1 then u
  when 2, 3 then u
  end # error: Expected `String` but found `NilClass` for method result type
end

sig {params(x: Integer, u: T.untyped).returns(String)}
def case_when_single_line(x, u)
  case x when 1 then u end # error: Expected `String` but found `NilClass` for method result type
end

sig {params(x: Integer, u: T.untyped).returns(String)}
def case_in(x, u)
  case x
  in 1 then u
  end
end

sig {params(c: T::Boolean, u: T.untyped).returns(String)}
def if_empty_else(c, u)
  if c
    u
  else
  end # error: Expected `String` but found `NilClass` for method result type
end

sig {params(c: T::Boolean, u: T.untyped).returns(String)}
def if_empty_then(c, u)
  if c
  else
    u
  end # error: Expected `String` but found `NilClass` for method result type
end

sig {params(c: T::Boolean, u: T.untyped).returns(String)}
def unless_empty_else(c, u)
  unless c
    u
  else
  end # error: Expected `String` but found `NilClass` for method result type
end

sig {params(c: T::Boolean, u: T.untyped).returns(String)}
def unless_empty_body(c, u)
  unless c
  else
    u
  end # error: Expected `String` but found `NilClass` for method result type
end

sig {params(x: Integer, u: T.untyped).returns(String)}
def case_when_empty_else(x, u)
  case x
  when 1 then u
  else
  end # error: Expected `String` but found `NilClass` for method result type
end

sig {params(c: T::Boolean).returns(String)}
def both_empty(c)
  if c
  end
#    ^ error: Expected `String` but found `NilClass` for method result type
#    ^ error: Expected `String` but found `NilClass` for method result type
end

sig {params(c: T::Boolean, d: T::Boolean, u: T.untyped).returns(String)}
def else_nested_if(c, d, u)
  if c
    u
  else
    if d
      u
    end
  end # error: Expected `String` but found `NilClass` for method result type
end

sig {params(c: T::Boolean, d: T::Boolean, u: T.untyped).returns(String)}
def nested_missing_else(c, d, u)
  if c
    if d
      u
    end # error: Expected `String` but found `NilClass` for method result type
  else
    u
  end
end

sig {params(c: T::Boolean, d: T::Boolean, u: T.untyped).returns(String)}
def both_missing_else(c, d, u)
  if c
    if d
      u
    end # error: Expected `String` but found `NilClass` for method result type
  end # error: Expected `String` but found `NilClass` for method result type
end

sig {params(c: T::Boolean).returns(String)}
def heredoc_value(c)
  if c
    <<~EOS
      end
    EOS
  end # error: Expected `String` but found `NilClass` for method result type
end

sig {params(c: T::Boolean).returns(Integer)}
def error_in_then_too(c)
  if c
    "not an integer" # error: Expected `Integer` but found `String("not an integer")` for method result type
  end # error: Expected `Integer` but found `NilClass` for method result type
end

sig {params(c: T::Boolean, u: T.untyped).returns(String)}
def modifier_if(c, u)
  u if c # error: Expected `String` but found `NilClass` for method result type
end

sig {params(c: T::Boolean, u: T.untyped).returns(String)}
def modifier_unless(c, u)
  u unless c # error: Expected `String` but found `NilClass` for method result type
end

sig {params(c: T::Boolean, d: T::Boolean, u: T.untyped).returns(String)}
def modifier_if_nested(c, d, u)
  u if d if c
#       ^ error: Expected `String` but found `NilClass` for method result type
#            ^ error: Expected `String` but found `NilClass` for method result type
end

sig {params(c: T::Boolean).returns(String)}
def modifier_if_return(c)
  return "" if c # error: Expected `String` but found `NilClass` for method result type
end

sig {params(c: T::Boolean, u: T.untyped).returns(String)}
def modifier_if_begin(c, u)
  begin
    u
  end if c # error: Expected `String` but found `NilClass` for method result type
end
