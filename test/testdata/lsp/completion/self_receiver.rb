# typed: true

def foo(xyz)
  self.
  #    ^ completion: CSV, ...
end # error: unexpected token

class A
  def foo
    self.y # error: does not exist
    #     ^ completion: yield_self, ...
  end
end
