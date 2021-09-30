# typed: true

class Parent
  def super(x)
  end

  def nullary; end

  def unary(x); end
end

class Child < Parent
  def nullary
    super # error: Not enough arguments provided for method `Parent#super`. Expected: `1`, got: `0`
  end

  def unary(x)
    super(1, 2) # error: Not enough arguments provided for method `Parent#super`. Expected: `1`, got: `2`
  end
end
