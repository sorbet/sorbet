# typed: true
extend T::Sig

sig {params(c: T::Boolean, u: T.untyped).returns(Integer)}
def missing_else(c, u)
  if c
    u
  end # error: Expected `Integer` but found `NilClass` for method result type
end

sig {params(c: T::Boolean, u: T.untyped).returns(Integer)}
def unless_missing_else(c, u)
  unless c
    u
  end # error: Expected `Integer` but found `NilClass` for method result type
end

sig {params(x: T.any(Integer, String, Symbol), u: T.untyped).returns(Integer)}
def case_missing_else(x, u)
  case x
  when Integer then x
  when String then u
  end # error: Expected `Integer` but found `NilClass` for method result type
end

sig {params(x: T.untyped).returns(Integer)}
def safe_navigation(x)
  x&.foo # error: Expected `Integer` but found `NilClass` for method result type
end

sig {params(x: T.nilable(String)).returns(Integer)}
def safe_navigation_typed(x)
  x&.length # error: Expected `Integer` but found `NilClass` for method result type
end
