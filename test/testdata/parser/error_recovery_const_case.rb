# typed: false

def foo(x)
  Integer::
  case x # error: unexpected token
  when Integer
  end
end # error: unexpected token
