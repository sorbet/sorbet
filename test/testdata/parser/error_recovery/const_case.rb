# typed: false

def foo(x)
  Integer::
 #       ^^ error: expected constant name following "::"
  case x
  when Integer
  end
end
