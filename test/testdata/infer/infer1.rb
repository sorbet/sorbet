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
  b = a.getbyte(a) # error: wrong argument type
end

def baz2
  a = "foo"
  b = a.getbyte("foo") # error: wrong argument type
end

def baz3
  b = "foo".getbyte("foo") # error: wrong argument type
end

def baz4
  b = a.getbyte("foo") # error: wrong reciever type
end

def baz5
 if (true)
   b = 1
 else
   b = "foo"
 end
 b = b.getbyte(b) # error: wrong reciever type
end

def baz6
 if (true)
   b = 1
 else
   b = "foo"
 end
 b = "foo".getbyte(b) # error: wrong argument type
end

def baz7
 if (true)
   b = 1
 end
 b = "foo".getbyte(b) # error: wrong argument type type
end


def baz8
 while (true)
   b = 1 # error: cant widen type
 end
end

