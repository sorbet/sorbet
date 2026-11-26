# typed: true
extend T::Sig

sig {params(c: T::Boolean, u: T.untyped).returns(String)}
def if_multi_line(c, u)
  if c
    u
  end
end

sig {params(c: T::Boolean, u: T.untyped).returns(String)}
def if_single_line(c, u)
  if c then u end
end

sig {params(x: Integer, u: T.untyped).returns(String)}
def elsif_chain(x, u)
  if x == 1
    u
  elsif x == 2
    u
  end
end

sig {params(c: T::Boolean, u: T.untyped).returns(String)}
def unless_multi_line(c, u)
  unless c
    u
  end
end

sig {params(x: Integer, u: T.untyped).returns(String)}
def case_when(x, u)
  case x
  when 1 then u
  when 2
    u
  end
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
  end
end

sig {params(c: T::Boolean, d: T::Boolean, u: T.untyped).returns(String)}
def both_missing_else(c, d, u)
  if c
    if d
      u
    end
  end
end

sig {params(c: T::Boolean).returns(Integer)}
def error_in_then_too(c)
  if c
    "not an integer"
  end
end

sig {params(c: T::Boolean, u: T.untyped).returns(String)}
def modifier_if(c, u)
  u if c
end

sig {params(c: T::Boolean, u: T.untyped).returns(String)}
def modifier_unless(c, u)
  u unless c
end

sig {params(c: T::Boolean, d: T::Boolean, u: T.untyped).returns(String)}
def modifier_if_nested(c, d, u)
  u if d if c
end

sig {params(c: T::Boolean).returns(String)}
def modifier_if_return(c)
  return "" if c
end

module M
  extend T::Sig

  sig {params(c: T::Boolean, u: T.untyped).returns(String)}
  def in_module(c, u)
    if c
      u
    end
  end
end

class B < BasicObject
  extend T::Sig

  sig {params(c: T::Boolean, u: T.untyped).returns(String)}
  def in_basic_object(c, u)
    if c
      u
    end
  end
end
