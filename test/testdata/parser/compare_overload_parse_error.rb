# typed: false
class A # error-with-dupes: Parse Error: class definition in method body
  def greater_eqaul>=(foo) # error-with-dupes: Parse Error: unexpected token: syntax error
  end
end

class B # error-with-dupes: Parse Error: class definition in method body
  def less_equal<=(foo) # error-with-dupes: Parse Error: unexpected token: syntax error
  end
end

class C # error-with-dupes: Parse Error: class definition in method body
  def not_equal!=(foo) # error-with-dupes: Parse Error: unexpected token: syntax error
  end
end
