# @typed
class ControlFlow

  sig(a: T.any(Integer, NilClass)).returns(Integer)
  def orZero0(a)
    if (a)
      return a
    else
      return 0
    end
  end

  sig(a: Integer).returns(Integer)
  def orZero0a(a)
    if (a)
      return a
    else
      return 0 # error: This code is unreachable
    end
  end

   sig(a: T.any(Integer, NilClass)).returns(Integer)
    def orZero0n(a)
      b = !a
      if (b)
        return 0
      else
        return a
      end
    end

  sig(a: T.any(Integer, NilClass)).returns(Integer)
  def orZero1n(a)
      b = !a.is_a?(Integer)
      if (b)
        return 0
      else
        return a
      end
    end

  sig(a: T.any(Integer, NilClass)).returns(Integer)
  def orZero2(a)
    a ||= 0;
    a
  end

  sig(a: T.any(Integer, NilClass)).returns(Integer)
  def orZero3(a)
      if (a && 1 == 2)
         return 1
      else
         return 0
      end
  end

  sig(a: T.any(Integer, NilClass)).returns(Integer)
  def orZero3n(a)
      b = !(a && 1 == 2)
      if (b)
         return 0
      else
         return 1
      end
  end

  sig(a: T.any(Integer, NilClass)).returns(Integer)
  def orZero4(a)
      if (a || true)
         return a # error: does not conform to method result type
      else
         return 0 # error: This code is unreachable
      end
  end

    sig(a: T.any(Integer, NilClass)).returns(Integer)
    def orZero5(a)
        if (a && true)
           return a
        else
           return 0
        end
    end

end
