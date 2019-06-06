
# typed: true
class Parent
  def self.foo; end
end

Class.new
Class.new(Parent)

Class.new {|cls| cls.superclass}
Class.new(Parent) {|cls| cls.superclass}

# Our ClassNew DSL pass can only re-write Class.new where the lefthand
# side is assigned to a constant
Class.new(Parent).foo # error: Method `foo` does not exist on `Class`
c = Class.new(Parent)
c.foo # error: Method `foo` does not exist on `Class`

C = Class.new(Parent) do |cls|
  cls.foo
  foo
end
C.foo
