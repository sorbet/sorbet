# typed: true

class Parent
  def super(x)
  end

  def nullary; end

  def unary(x); end
end

class Child < Parent
  def nullary
    super
  end

  def unary(x)
    super(1, 2)
    #        ^ error: Too many arguments provided for method `Parent#unary`
  end
end
