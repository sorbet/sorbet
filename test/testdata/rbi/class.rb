# typed: true

class Parent
  def self.foo; end
end

Parent.singleton_class.attached_object
Parent.attached_object
Parent.new.singleton_class.attached_object

c1 = Class.new
T.reveal_type(c1) # error: Revealed type: `T::Class[Object]`
T.reveal_type(c1.new) # error: Revealed type: `Object`

c2 = Class.new(Parent)
T.reveal_type(c2) # error: Revealed type: `T.class_of(Parent)`
T.reveal_type(c2.new) # error: Revealed type: `Parent`

c3 = Class.new { |cls| cls.superclass }
T.reveal_type(c3) # error: Revealed type: `T::Class[Object]`
T.reveal_type(c3.new) # error: Revealed type: `Object`

c4 = Class.new(Parent) { |cls| cls.superclass }
T.reveal_type(c4) # error: Revealed type: `T.class_of(Parent)`
T.reveal_type(c4.new) # error: Revealed type: `Parent`

# Our ClassNew Rewriter pass can only re-write Class.new where the lefthand
# side is assigned to a constant
Class.new(Parent).foo
c = Class.new(Parent)
c.foo
c.new.foo # error: Method `foo` does not exist on `Parent`

C = Class.new(Parent) do |cls|
  cls.foo
  foo
end
C.foo

Class.new('Foo') # error: Expected `T.all(T::Class[T.anything], T.type_parameter(:Parent))` but found `String("Foo")` for argument `super_class`
