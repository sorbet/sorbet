class String
  standard_method(
    {
      i: Integer,
    },
    returns: Opus::Types.any(Integer, NilClass)
  )
  def getbyte(i)
  end
end

class Integer
  standard_method(
    {
      o: Integer,
    },
    returns: Opus::Types.any(Integer)
  )
  def +(o)
  end
end

def baz1
  a = "foo"
  b = a.getbyte(a) # error: does not match expected type
end

def baz2
  a = "foo"
  b = a.getbyte("foo") # error: does not match expected type
end

def baz3
  b = "foo".getbyte("foo") # error: does not match expected type
end

def baz4
  b = a.getbyte("foo") # error: does not exist
end

def baz5
 if (true)
   b = 1
 else
   b = "foo"
 end
 b = b.getbyte(1) # error: does not exist
end

def baz6
 if (true)
   b = 1
 else
   b = "foo"
 end
 b = "foo".getbyte(b) # error: does not match expected type
end

def baz7
 if (true)
   b = 1
 end
 b = "foo".getbyte(b) # error: does not match expected type
end


def baz8
 while (true)
   b = 1
 end
end

