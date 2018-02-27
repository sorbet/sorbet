class A
  sig(x: T.any(String, Integer), y: Integer).returns(Integer)
  def a_method(x, y)
    if x.is_a?(String)
      x = x.to_i
    end
    x + y
  end
end
