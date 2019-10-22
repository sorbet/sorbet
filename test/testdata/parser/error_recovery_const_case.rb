# typed: false

def foo(x)
  Integer::
  case x
  when Integer # error: unexpected token
  end
end # error: unexpected token
