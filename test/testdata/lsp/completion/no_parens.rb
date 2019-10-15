# typed: true

class A
  def foo_1; end
end

A.new.foo_ # error: does not exist
#         ^ apply-completion: [A] item: 0
