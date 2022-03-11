# typed: true

def foo(xyz)
  self.
  #    ^ completion: CSV, ...
end # error: unexpected token
