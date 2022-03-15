# typed: true

module
  def foo(xyz)
    self.
    #    ^ completion: (nothing)
  end # error: unexpected token
end

class A
  def foo
    self.y # error: does not exist
    #     ^ completion: yield_self, ...
  end
end
