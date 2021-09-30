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
    # Reporting no error here is actually incorrect, but consistently incorrect
    #
    # (it doesn't have to do with conflating `super()` and `<super>()` but
    # rather that we don't type check `<super>` at all.
    # See https://github.com/sorbet/sorbet/issues/1068)
    super(1, 2)
  end
end
