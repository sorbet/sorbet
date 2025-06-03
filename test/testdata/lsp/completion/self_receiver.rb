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
    #     ^ completion: to_yaml, yield_self, Array, display, initialize_copy, pretty_inspect, syscall, system
  end
end
