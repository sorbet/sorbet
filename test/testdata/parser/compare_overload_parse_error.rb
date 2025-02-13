# typed: false
class A # parser-error: class definition in method body
  def greater_equal>=(foo) # parser-error: unexpected token ">="
  end
end

class B # parser-error: class definition in method body
  def less_equal<=(foo) # parser-error: unexpected token "<="
  end
end
