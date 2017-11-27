class ControlFlow

  standard_method({a: Opus::Types.any(Integer, NilClass)}, returns: Integer)
  def orZero(a)
    if (a)
      a
    else
      0
    end
  end

end
