# typed: strict
class FalseClass
  sig {params(obj: BasicObject).returns(FalseClass)}
  def &(obj)
  end
  sig {params(obj: BasicObject).returns(T::Boolean)}
  def ^(obj)
  end
  sig {params(obj: BasicObject).returns(T::Boolean)}
  def |(obj)
  end
  sig {returns(TrueClass)}
  def !
  end
end
