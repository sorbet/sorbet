# typed: true

class A
  def self.foo_1; end
  def self.foo_2; end

  foo # error: does not exist
#    ^ completion: foo_1, foo_2

  f # error: does not exist
#  ^ completion: foo
end
