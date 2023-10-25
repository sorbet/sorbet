# typed: true
class Foo
  self::Bar = Struct.new(:x)
end

a = Foo::Bar.new(1)
a.x = 2
a.y = 3 # error: Setter method `y=` does not exist on `Foo::Bar`
