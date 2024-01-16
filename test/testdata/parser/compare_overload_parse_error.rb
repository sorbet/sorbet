# typed: false
class A # error: class definition in method body
  def greater_equal>=(foo) # error: unexpected token ">="
  end
end

class B # error: class definition in method body
  def less_equal<=(foo) # error: unexpected token "<="
  end
end
