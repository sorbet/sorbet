# typed: true

class A
  def foo; end
end

A.new. # error: Parse Error
#     ^ completion: foo, ...
