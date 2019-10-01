# typed: true

class A
  def foo; end
end

A.new. # error: unexpected token $end
#     ^ completion: foo, ...
