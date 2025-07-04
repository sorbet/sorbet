# typed: false

def foo(x)
  Integer::
 #       ^^ parser-error: expected constant name following "::"
  case x
  when Integer
  end
end
