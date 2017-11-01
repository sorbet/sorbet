class String1
  standard_method(
    {
      i: Integer,
    },
    returns: Opus::Types.any(Integer, NilClass)
  )
  def getbyte(i)
  end
end

class Integer1
  standard_method(
    {
      o: Integer,
    },
    returns: Opus::Types.any(Integer)
  )
  def +(o)
  end
end

def baz
  a = "foo"
  b = a.getbyte(1)
  2 + b
end
