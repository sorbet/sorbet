class ControlFlow

  standard_method({a: Opus::Types.any(Integer, NilClass)}, returns: Integer)
  def orZero(a)
    if (a)
      return a #error: does not conform to method result type
    else
      return 0
    end
  end

end
