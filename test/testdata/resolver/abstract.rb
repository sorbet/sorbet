# @typed

class HasAbstract
  sig(x: Integer).abstract.returns(String)
  def abstract(x)
  end

  sig(x: Integer).abstract.returns(String)
  def abstract_with_body(x)
    14 # error: Abstract methods must not contain any code in their body.
  end

end
