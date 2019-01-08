# typed: strict
class FalseClass
  sig {params(obj: BasicObject).returns(FalseClass)}
  def &(obj)
  end
  sig {params(obj: BasicObject).returns(T.any(FalseClass, TrueClass))}
  def ^(obj)
  end
  sig {params(obj: BasicObject).returns(T.any(FalseClass, TrueClass))}
  def |(obj)
  end
end
