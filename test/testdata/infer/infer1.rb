# typed: true
def baz1
  a = "foo"
  b = a.getbyte(a) # error: Expected `Integer` but found `String("foo")` for argument `arg0`
end

def baz2
  a = "foo"
  b = a.getbyte("foo") # error: Expected `Integer` but found `String("foo")` for argument `arg0`
end

def baz3
  b = "foo".getbyte("foo") # error: Expected `Integer` but found `String("foo")` for argument `arg0`
end

def baz4
  b = a.getbyte("foo") # error: does not exist
end

def baz5(cond)
 if (cond)
   b = 1
 else
   b = "foo"
 end
 b = b.getbyte(1) # error: does not exist
end

def baz6(cond)
 if (cond)
   b = 1
 else
   b = "foo"
 end
 b = "foo".getbyte(b) # error: Expected `Integer` but found `T.any(String, Integer)` for argument `arg0`
end

def baz7(cond)
 if (cond)
   b = 1
 end
 b = "foo".getbyte(b) # error: Expected `Integer` but found `T.nilable(Integer)` for argument `arg0`
end


def baz8
 while (true)
   b = 1
 end
end

