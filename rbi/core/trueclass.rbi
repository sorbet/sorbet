# typed: strict
class TrueClass
  sig {params(obj: BasicObject).returns(T.any(FalseClass, TrueClass))}
  def &(obj)
  end
  sig {params(obj: BasicObject).returns(T.any(FalseClass, TrueClass))}
  def ^(obj)
  end
  sig {params(obj: BasicObject).returns(TrueClass)}
  def |(obj)
  end
end
