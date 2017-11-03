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
  b = a.getbyte(a) # wrong argument type
end

def baz2
  a = "foo"
  b = a.getbyte("foo") # wrong argument type
end

def baz3
  b = "foo".getbyte("foo") # wrong argument type
end

def baz4
  b = a.getbyte("foo") # wrong reciever type
end

def baz5
 if (true)
   b = 1
 else
   b = "foo"
 end
 b = b.getbyte(b) # wrong reciever type
end

def baz6
 if (true)
   b = 1
 else
   b = "foo"
 end
 b = "foo".getbyte(b) # wrong argument type
end

def baz7
 if (true)
   b = 1
 end
 b = "foo".getbyte(b) # wrong argument type type
end


def baz8
 while (true)
   b = 1 # cant widen type
 end
end

