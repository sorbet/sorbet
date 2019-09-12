# typed: true

class A
  def foo_1; end
  def foo_2; end
end

A.new.foo # error: does not exist
#        ^ completion: foo_1, foo_2
#        ^ completion: foo_2, ...
