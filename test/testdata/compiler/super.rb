# typed: true
class A
  def no_args(x)
    x
  end
  def same_arg(x)
    x
  end
  def fixed_arg(x)
    x
  end
  def parens
    4
  end
end

class B < A
  def no_args(x)
    super
  end
  def same_arg(x)
    super(x)
  end
  def fixed_arg
    super(3)
  end
  def parens
    super()
  end
end

# puts B.new.no_args(1)
puts B.new.same_arg(2)
puts B.new.fixed_arg
puts B.new.parens
